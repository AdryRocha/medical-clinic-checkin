#include "board_factory.hpp"
#include "pin_config.hpp"
#include "services/logger_service.hpp"
#include "pico/stdlib.h"

// HALs - Mantendo sua estrutura rica
#include "hal/rp2350/hal_gpio_rp2350.hpp"
#include "hal/rp2350/hal_spi_rp2350.hpp"
#include "hal/rp2350/hal_i2c_rp2350.hpp"
#include "hal/rp2350/hal_uart_rp2350.hpp"
#include "hal/rp2350/hal_time_rp2350.hpp"
#include "hal/rp2350/hal_wifi_rp2350.hpp"
#include "utils/ring_buffer.hpp"

// Definições de fallback caso não estejam no pin_config.hpp
#ifndef TOUCH_PIN_SDA
#define TOUCH_PIN_SDA 6
#endif
#ifndef TOUCH_PIN_SCL
#define TOUCH_PIN_SCL 7
#endif

BoardContext BoardFactory::create_pico2w() {
    BoardContext ctx{};

    // ===== DIAGNOSTICS & PIN CONFLICT CHECK =====
    Logger::info("\n[FACTORY] ========== HARDWARE DIAGNOSTICS ==========");
    Logger::info("[FACTORY] Display (ST7796) - SPI0");
    Logger::info("  SCK=18  MOSI=19  MISO=16  CS=17  DC=20  RST=21");
    Logger::info("[FACTORY] Touch (FT6336U) - I2C1");
    Logger::info("  SDA=6  SCL=7  RST=8  IRQ=9 (Addr=0x38)");
    Logger::info("[FACTORY] ==========================================");
    
    // Check for pin conflicts (use uint32_t to avoid overflow)
    const uint32_t spi_pins = (1U << 18) | (1U << 19) | (1U << 16) | (1U << 17) | (1U << 20) | (1U << 21);
    const uint32_t i2c_pins = (1U << 6) | (1U << 7) | (1U << 8) | (1U << 9);
    if (spi_pins & i2c_pins) {
        Logger::error("[FACTORY] PIN CONFLICT DETECTED! SPI and I2C share pins!");
    } else {
        Logger::info("[FACTORY] Pin mapping: OK (no conflicts)");
    }

    // ===== DISPLAY HARD RESET (RST=21) =====
    gpio_init(21);
    gpio_set_dir(21, GPIO_OUT);
    gpio_put(21, 0);
    sleep_ms(100);
    gpio_put(21, 1);
    sleep_ms(100);
    Logger::info("[FACTORY] Display hardware reset complete.");

    // ===== I2C1 INITIALIZATION (TOUCH BUS) =====
    Logger::info("[FACTORY] Initializing I2C1 (SDA=6, SCL=7) @ 100kHz...");
    static HAL_I2C_RP2350 i2c_touch_hal(i2c1, 6, 7);
    if (!i2c_touch_hal.init(100000)) {
        Logger::error("[FACTORY] I2C1 init failed!");
    } else {
        Logger::info("[FACTORY] I2C1 initialized with internal pull-ups (50k)");
        Logger::info("[FACTORY] CRITICAL: External 4.7kΩ pull-ups to 3.3V required per RP2350 Erratum E9");
    }
    ctx.i2c = &i2c_touch_hal;

    static HAL_GPIO_RP2350 gpio_hal;
    ctx.gpio = &gpio_hal;

    // ===== UART0 INITIALIZATION (QR SCANNER) =====
    static HAL_UART_RP2350 uart_qr_hal(QR_UART_ID, QR_SCANNER_PIN_TX, QR_SCANNER_PIN_RX, UART0_IRQ, nullptr);
    uart_qr_hal.init(QR_BAUDRATE);
    ctx.uart_qr = &uart_qr_hal;

    // ===== UART1 INITIALIZATION (FINGERPRINT) =====
    static uint8_t fp_rb_storage[256];
    static RingBuffer fp_ring_buffer(fp_rb_storage, sizeof(fp_rb_storage));
    static HAL_UART_RP2350 uart_fp_hal(FP_UART_ID, FINGERPRINT_PIN_TX, FINGERPRINT_PIN_RX, UART1_IRQ, &fp_ring_buffer);
    uart_fp_hal.init(FP_BAUDRATE);
    ctx.uart_fingerprint = &uart_fp_hal;
    ctx.fingerprint_buffer = &fp_ring_buffer;

    // ===== SPI0 INITIALIZATION (DISPLAY BUS) =====
    Logger::info("[FACTORY] Initializing SPI0 (Display) @ %u Hz...", (unsigned)DISPLAY_BAUDRATE);
    Logger::info("  SCK=18  MOSI=19  MISO=16  CS=17  DC=20  RST=21");
    static HAL_SPI_RP2350 spi_disp_hal(spi0, 18, 19, 16, 17, 20, 21);
    spi_disp_hal.init(DISPLAY_BAUDRATE);
    ctx.spi_display = &spi_disp_hal;
    Logger::info("[FACTORY] SPI0 configured (ready for init() call)");
    
    Logger::info("[FACTORY] Board initialization complete.\n");
    return ctx;
}