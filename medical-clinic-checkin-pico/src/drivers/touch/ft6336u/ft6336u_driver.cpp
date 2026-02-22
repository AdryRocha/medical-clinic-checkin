#include "ft6336u_driver.hpp"
#include "services/logger_service.hpp"
#include "pin_config.hpp"
#include "display_config.hpp"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

// Registradores Internos
#define REG_MODE        0x00
#define REG_TD_STATUS   0x02
#define REG_TOUCH1_XH   0x03
#define REG_CHIP_ID     0xA3 // Registrador onde mora o ID real (0x64 ou 0xA3)

FT6336U_Driver::FT6336U_Driver(HAL_I2C_Interface* i2c, HAL_GPIO_Interface* gpio, 
                               uint8_t address, uint8_t irq_pin, uint8_t reset_pin)
    : i2c_(i2c), gpio_(gpio), i2c_address_(address), 
      irq_(irq_pin), rst_(reset_pin), initialized_(false)
{
    last_point_.x = 0; last_point_.y = 0; last_point_.valid = false;
}

void FT6336U_Driver::resetChip() {
    if (rst_ != 0xFF && gpio_) {
        gpio_->init(rst_, PinDirection::Output);
        // Sequência compatível com módulos FT6336U: HIGH -> LOW -> HIGH
        gpio_->write(rst_, 1);
        sleep_ms(10);
        gpio_->write(rst_, 0);
        sleep_ms(20);
        gpio_->write(rst_, 1);
        sleep_ms(300);
    }
}

bool FT6336U_Driver::init() {
    if (irq_ != 0xFF) {
        if (gpio_) {
            gpio_->init(irq_, PinDirection::Input);
        } else {
            gpio_init(irq_);
            gpio_set_dir(irq_, GPIO_IN);
        }
        gpio_pull_up(irq_);
    }

    Logger::info("[FT6336U] Iniciando reset de hardware (Pino %d)...", rst_);
    resetChip(); // 20ms LOW + 300ms HIGH

    // Garante modo de operação normal (alguns módulos exigem set explícito)
    if (i2c_) {
        if (!i2c_->write_reg(i2c_address_, REG_MODE, 0x00)) {
            Logger::warn("[FT6336U] Falha ao setar REG_MODE=0x00");
        }
    }

    uint8_t chip_id = 0;
    // GAP 4: Leitura REAL do registrador de ID 0xA3 com validação
    if (readRegister(REG_CHIP_ID, &chip_id)) {
        if (chip_id == 0x64) {
            Logger::info("[FT6336U] Sensor validado com SUCESSO! ID: 0x64.");
            initialized_ = true;
            return true;
        } else {
            Logger::error("[FT6336U] Erro: ID detectado (0x%02X) diferente de 0x64.", chip_id);
        }
    } else {
        Logger::error("[FT6336U] Falha crítica de comunicação I2C1 (0x38).");
    }
    return false;
}

// CORREÇÃO: Função definida apenas UMA vez para evitar erro de redefinição
bool FT6336U_Driver::isTouched() { 
    if (!initialized_) return false;
    uint8_t status = 0;
    if (!readRegister(REG_TD_STATUS, &status)) return false;
    return (status & 0x0F) > 0;
}

bool FT6336U_Driver::readRegister(uint8_t reg, uint8_t* value) {
    if (!i2c_ || !value) return false;
    return i2c_->write_read(i2c_address_, &reg, 1, value, 1);
}

uint8_t FT6336U_Driver::getTdStatus() {
    uint8_t status = 0xFF;
    if (!readRegister(REG_TD_STATUS, &status)) {
        return 0xFF;
    }
    return status;
}

bool FT6336U_Driver::readRegisters(uint8_t start_reg, uint8_t* buffer, size_t len) {
    if (!i2c_ || !buffer || len == 0) return false;
    return i2c_->write_read(i2c_address_, &start_reg, 1, buffer, len);
}

bool FT6336U_Driver::readRawPoint(uint16_t& x, uint16_t& y, bool& touched) {
    uint8_t status;
    if (!readRegister(REG_TD_STATUS, &status)) {
        static uint32_t last_err_ms = 0;
        uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        if (now_ms - last_err_ms > 1000u) {
            Logger::warn("[DRIVER] Falha ao ler REG_TD_STATUS (I2C)");
            last_err_ms = now_ms;
        }
        return false;
    }

    // Log periódico do REG_TD_STATUS para diagnóstico (1x por segundo)
    static uint32_t last_status_log_ms = 0;
    uint32_t now_ms = to_ms_since_boot(get_absolute_time());
    if (now_ms - last_status_log_ms >= 1000u) {
        Logger::info("[DRIVER] REG_TD_STATUS=0x%02X", status);
        last_status_log_ms = now_ms;
    }

    uint8_t point_count = status & 0x0F;

    // --- ADICIONE ESTE LOG PARA VER SE O CHIP DETECTA PRESSÃO ---
    if (point_count > 0) {
        Logger::info("[DRIVER] Chip detectou %d pontos de toque", point_count);
    }
    // -----------------------------------------------------------

    if (point_count == 0 || point_count > 2) { touched = false; return true; }
    uint8_t buf[4];
    if (!readRegisters(REG_TOUCH1_XH, buf, 4)) {
        static uint32_t last_err_ms2 = 0;
        uint32_t now_ms2 = to_ms_since_boot(get_absolute_time());
        if (now_ms2 - last_err_ms2 > 1000u) {
            Logger::warn("[DRIVER] Falha ao ler dados de toque (I2C)");
            last_err_ms2 = now_ms2;
        }
        return false;
    }
    
    x = static_cast<uint16_t>(((buf[0] & 0x0F) << 8) | buf[1]);
    y = static_cast<uint16_t>(((buf[2] & 0x0F) << 8) | buf[3]);
    
    touched = true;

    // --- LOG DE DADOS BRUTOS ---
    Logger::info("[DRIVER] Toque Detectado! Raw X: %u, Raw Y: %u", x, y);
    // ---------------------------

    return true;
}

bool FT6336U_Driver::readPoint(TouchPoint* point) {
    if (!point || !initialized_) return false;
    uint16_t x, y; bool touched;
    if (!readRawPoint(x, y, touched)) return false;
    if (!touched) { point->valid = false; return false; }

    // Rotaciona nos eixos brutos 12-bit (0..4095) para alinhar com a rotação do display.
    // Deixa o escalonamento e clamp para o adaptador LVGL.
    constexpr uint16_t raw_max = 4095; // FT6336U é 12-bit
    uint16_t adj_x = x;
    uint16_t adj_y = y;

    switch (DISP_ROTATION % 4) {
        case 0: // Portrait
            break;
        case 1: // Landscape 90° CW
            adj_x = y;
            adj_y = raw_max - x;
            break;
        case 2: // Portrait 180°
            adj_x = raw_max - x;
            adj_y = raw_max - y;
            break;
        case 3: // Landscape 270°
            adj_x = raw_max - y;
            adj_y = x;
            break;
    }

    point->x = adj_x;
    point->y = adj_y;
    point->valid = true;
    return true;
}