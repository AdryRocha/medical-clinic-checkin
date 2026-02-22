#pragma once

#include <cstdint>
#include "drivers/touch/interface/touch_interface.hpp"
#include "hal/interfaces/hal_i2c_interface.hpp"
#include "hal/interfaces/hal_gpio_interface.hpp"

class FT6336U_Driver : public TouchInterface {
public:
    FT6336U_Driver(HAL_I2C_Interface* i2c,
                   HAL_GPIO_Interface* gpio,
                   uint8_t i2c_address = 0x38,
                   uint8_t irq_pin = 0xFF,
                   uint8_t reset_pin = 0xFF);

    bool init() override;
    bool readPoint(TouchPoint* point) override;
    bool isTouched() override;

    // Debug: lê REG_TD_STATUS diretamente
    uint8_t getTdStatus();

    // Métodos extras para debug (da versão antiga)
    uint8_t getChipID();
    uint8_t getFirmwareVersion();

private:
    void resetChip();

    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t start_reg, uint8_t* buffer, size_t len);
    bool readRawPoint(uint16_t& x, uint16_t& y, bool& touched);

    HAL_I2C_Interface* i2c_;
    HAL_GPIO_Interface* gpio_;
    uint8_t i2c_address_;
    uint8_t irq_;
    uint8_t rst_;

    bool initialized_;
    TouchPoint last_point_; // Guarda o último ponto para evitar "pulos"
    bool last_valid_ = false;
};