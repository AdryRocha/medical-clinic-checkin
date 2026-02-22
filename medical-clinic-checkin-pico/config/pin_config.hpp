#ifndef PIN_CONFIG_HPP
#define PIN_CONFIG_HPP

// #include "pico/stdlib.h"

/**
 * @file pin_config.hpp
 * @brief Hardware pin configuration for the Medical Clinic Check-in System
 * 
 * This file centralizes all GPIO pin assignments for the Raspberry Pi Pico
 * to make it easier to change hardware connections without modifying code.
 */

// ============================================================================
// SPI Display Pins (ST7796)  (Usa SPI0)
// ============================================================================
#define DISPLAY_SPI_ID   spi0
#define DISPLAY_PIN_MOSI 19  // SPI0 TX (Master Out Slave In)
#define DISPLAY_PIN_SCK  18  // SPI0 Clock
#define DISPLAY_PIN_CS   17  // Chip Select
#define DISPLAY_PIN_DC   20  // Data/Command select
#define DISPLAY_PIN_RST  21  // Display Reset

// Backlight (Opcional, se for usar controle de brilho)
#define DISPLAY_PIN_BL   22 // Backlight control pin (pino físico 29)

// ============================================================================
// I2C Touch Controller Pins (FT6336U)  (Usa I2C1)
// ============================================================================
#define TOUCH_I2C_ID     i2c1
#define TOUCH_PIN_RST 8  // Touch controller Reset
#define TOUCH_PIN_INT 9  // Touch controller Interrupt

// ============================================================================
// QR Scanner (GM67) Pins  (Usa UART0)
// ============================================================================
#define QR_UART_ID        uart0
#define QR_SCANNER_PIN_TX 0  // Conecta no RX do GM67
#define QR_SCANNER_PIN_RX 1  // Conecta no TX do GM67

// ============================================================================
// Fingerprint Sensor (R307S) Pins (Usa UART1)
// ============================================================================
#define FP_UART_ID         uart1
#define FINGERPRINT_PIN_TX 4  // Conecta no RX do R307
#define FINGERPRINT_PIN_RX 5  // Conecta no TX do R307

// ============================================================================
// SD Card Reader Pins (Usa SPI1)
// ============================================================================
// Nota: O serviço de storage precisa ser configurado para usar SPI1 e estes pinos
#define SDCARD_SPI_ID     spi1
#define SDCARD_PIN_MISO 12  // SPI1 RX (Master In Slave Out) - GP12
#define SDCARD_PIN_CS   13  // Chip Select - GP13
#define SDCARD_PIN_SCK  14  // SPI1 Clock - GP14
#define SDCARD_PIN_MOSI 15  // SPI1 TX (Master Out Slave In) - GP15

// Baudrates
#define DISPLAY_BAUDRATE    6000000  // 6 MHz para estabilidade inicial
#define SD_BAUDRATE         4000000 // 4 MHz (Seguro para SPI Mode)
#define QR_BAUDRATE         9600    // Padrão GM67
#define FP_BAUDRATE         57600    // Padrão R307S
#define TOUCH_BAUDRATE      400000   // 400 kHz (Fast Mode)

#endif // PIN_CONFIG_HPP
