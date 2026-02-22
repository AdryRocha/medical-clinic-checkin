#pragma once
#include "lvgl.h"
#include "drivers/touch/interface/touch_interface.hpp"

class LVGLTouchAdapter {
public:
    // Tipos devem ser uint16_t para coincidir com o display
    LVGLTouchAdapter(TouchInterface* touch, uint16_t screen_width, uint16_t screen_height);
    void registerInputDevice();

private:
    // LVGL exige um callback estático
    static void read_cb(lv_indev_drv_t* drv, lv_indev_data_t* data);

    TouchInterface* touch_;  // O erro indicou que o nome correto é touch_
    uint16_t width_;
    uint16_t height_;
    lv_indev_drv_t indev_drv_;
};