#ifndef LVGL_TOUCH_ADAPTER_HPP
#define LVGL_TOUCH_ADAPTER_HPP

#include "lvgl.h"
#include "drivers/touch/interface/touch_interface.hpp"

/**
 * @brief Adapter to connect LVGL with touch input drivers
 * 
 * This adapter bridges the gap between LVGL's input device interface
 * and the hardware touch driver. It handles coordinate transformation
 * for different display orientations.
 */
class LVGLTouchAdapter {
private:
    TouchInterface* touch_;
    lv_indev_drv_t indev_drv_;
    lv_indev_t* indev_;
    
    uint16_t display_width_;
    uint16_t display_height_;
    uint8_t rotation_;  // 0=0°, 1=90°, 2=180°, 3=270°
    
    TouchPoint last_point_;
    int16_t offset_x_;
    int16_t offset_y_;
    
    /**
     * @brief LVGL input read callback
     * Called by LVGL to read touch input state
     */
    static void readCallback(lv_indev_drv_t* drv, lv_indev_data_t* data);
    
    /**
     * @brief Transform touch coordinates based on display rotation
     * @param point Input touch point (in touch controller's native orientation)
     * @param x_out Transformed X coordinate for display
     * @param y_out Transformed Y coordinate for display
     */
    void transformCoordinates(const TouchPoint& point, int16_t& x_out, int16_t& y_out);

public:
    /**
     * @brief Initialize LVGL touch adapter
     * @param touch Pointer to touch driver implementing TouchInterface
     * @param display_width Display width in current orientation
     * @param display_height Display height in current orientation
     * @param rotation Display rotation (0=0°, 1=90°, 2=180°, 3=270°)
     */
    LVGLTouchAdapter(TouchInterface* touch,
                     uint16_t display_width,
                     uint16_t display_height,
                     uint8_t rotation = 0);
    
    /**
     * @brief Register touch input device with LVGL
     * @return Pointer to LVGL input device object
     */
    lv_indev_t* registerInputDevice();
    
    /**
     * @brief Update display rotation (call this if display orientation changes)
     * @param rotation New rotation value (0=0°, 1=90°, 2=180°, 3=270°)
     */
    void setRotation(uint8_t rotation);
};

#endif // LVGL_TOUCH_ADAPTER_HPP