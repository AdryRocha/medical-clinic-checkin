#ifndef DISPLAY_CONFIG_HPP
#define DISPLAY_CONFIG_HPP

/**
 * @file display_config.hpp
 * @brief Display hardware configuration for the Medical Clinic Check-in System
 * 
 * This file contains display resolution, orientation, and LVGL buffer settings.
 * Adjust these values based on your display hardware and performance requirements.
 */

// ============================================================================
// Display Hardware Configuration
// ============================================================================

// Display resolution (landscape mode)
#define DISP_HOR_RES 480  // Horizontal resolution in pixels
#define DISP_VER_RES 320  // Vertical resolution in pixels

// Display orientation
// 0 = Portrait (0°)
// 1 = Landscape (90° clockwise)
// 2 = Portrait inverted (180°)
// 3 = Landscape inverted (270° clockwise)
#define DISP_ROTATION 1   // Default: Landscape mode

// Display SPI speed
#define DISP_SPI_SPEED 10000000  // 20 MHz (ST7796 working speed)

// ============================================================================
// LVGL Buffer Configuration
// ============================================================================

// LVGL buffer size in lines
// Larger buffer = better performance but uses more RAM
// Recommended: 10-50 lines depending on available RAM
// Rule of thumb: buffer_size = DISP_HOR_RES * LVGL_BUFFER_LINES * sizeof(lv_color_t)
#define LVGL_BUFFER_LINES 5  // 5 lines: ~5KB per buffer (480 * 5 * 2 bytes) - optimized for memory

// Double buffering
// true  = Use 2 buffers (better performance, more RAM usage)
// false = Use 1 buffer (less RAM, may cause tearing on fast updates)
#define LVGL_USE_DOUBLE_BUFFER false

// ============================================================================
// Touch Controller Configuration
// ============================================================================

// Touch I2C instance
#define TOUCH_I2C_INSTANCE i2c1  // i2c0 or i2c1 depending on pins used

// Touch I2C address
#define TOUCH_I2C_ADDR 0x38  // FT6336U default address

// Touch I2C speed
#define TOUCH_I2C_SPEED 400000  // 400 kHz (fast mode)

// Touch controller dimensions (native/portrait orientation)
// Note: These are physical dimensions before rotation is applied
#define TOUCH_WIDTH  DISP_VER_RES  // 320 in portrait mode
#define TOUCH_HEIGHT DISP_HOR_RES  // 480 in portrait mode

#endif // DISPLAY_CONFIG_HPP