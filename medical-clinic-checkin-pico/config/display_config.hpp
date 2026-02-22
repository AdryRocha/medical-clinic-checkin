#ifndef DISPLAY_CONFIG_HPP
#define DISPLAY_CONFIG_HPP

// Display resolution (physical native orientation: portrait)
#define DISP_HOR_RES 480  // Horizontal resolution in pixels (portrait)
#define DISP_VER_RES 320  // Vertical resolution in pixels (portrait)

// Display orientation
// 0 = Portrait (0°)
// 1 = Landscape (90° clockwise)
// 2 = Portrait inverted (180°)
// 3 = Landscape inverted (270° clockwise)
#define DISP_ROTATION 1   // Use 0 if the panel is physically portrait

// Display SPI speed
#define DISP_SPI_SPEED 10000000  // 20 MHz (ST7796 working speed)

// LVGL Buffer Configuration
#define LVGL_BUFFER_LINES 5  // lines per buffer
#define LVGL_USE_DOUBLE_BUFFER false

// Touch I2C instance
#define TOUCH_I2C_INSTANCE i2c1  // i2c0 or i2c1 depending on pins used

// Touch Controller configuration (native dimensions in portrait)
#define TOUCH_I2C_ADDR 0x38   // FT6336U default address
#define TOUCH_I2C_SPEED 400000  // 400 kHz (fast mode) 

// Touch dimensions in native (portrait) orientation
#define TOUCH_WIDTH  DISP_HOR_RES  // 480 in portrait mode
#define TOUCH_HEIGHT DISP_VER_RES  // 320 in portrait mode

#endif // DISPLAY_CONFIG_HPP