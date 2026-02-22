#ifndef PIN_CONFIG_HPP
#define PIN_CONFIG_HPP

/**
 * @file pin_config.hpp
 * @brief Hardware pin configuration for the Medical Clinic Check-in System
 * 
 * This file centralizes all GPIO pin assignments for the Raspberry Pi Pico
 * to make it easier to change hardware connections without modifying code.
 */

// ============================================================================
// SPI Display Pins (ST7796)
// ============================================================================
#define DISPLAY_PIN_MOSI 19  // SPI0 TX (Master Out Slave In)
#define DISPLAY_PIN_SCK  18  // SPI0 Clock
#define DISPLAY_PIN_CS   17  // Chip Select
#define DISPLAY_PIN_DC   20  // Data/Command select
#define DISPLAY_PIN_RST  21  // Display Reset

// ============================================================================
// I2C Touch Controller Pins (FT6336U)
// ============================================================================
#define TOUCH_PIN_SDA 6  // I2C1 Data
#define TOUCH_PIN_SCL 7  // I2C1 Clock
#define TOUCH_PIN_RST 8  // Touch controller Reset
#define TOUCH_PIN_INT 9  // Touch controller Interrupt

// ============================================================================
// Status LED
// ============================================================================
#ifndef PICO_DEFAULT_LED_PIN
#define STATUS_PIN_LED 25  // Default onboard LED for Pico
#else
#define STATUS_PIN_LED PICO_DEFAULT_LED_PIN
#endif

// ============================================================================
// QR Scanner (GM67) Pins
// ============================================================================
// Uses UART0 (GP0, GP1)
// Debug output via USB (stdio_usb enabled in CMakeLists.txt)
#define QR_SCANNER_PIN_TX 0
#define QR_SCANNER_PIN_RX 1

// ============================================================================
// Fingerprint Sensor (R307S) Pins
// ============================================================================
// Uses UART1 (GP4, GP5)
#define FINGERPRINT_PIN_TX 4
#define FINGERPRINT_PIN_RX 5

// ============================================================================
// SD Card Reader Pins (SPI1)
// ============================================================================
#define SDCARD_PIN_MISO 12  // SPI1 RX (Master In Slave Out) - GP12
#define SDCARD_PIN_MOSI 15  // SPI1 TX (Master Out Slave In) - GP15
#define SDCARD_PIN_SCK  14  // SPI1 Clock - GP14
#define SDCARD_PIN_CS   13  // Chip Select - GP13

#endif // PIN_CONFIG_HPP
