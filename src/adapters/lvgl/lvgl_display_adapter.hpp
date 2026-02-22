#pragma once

#include "lvgl.h"
#include "../../drivers/display/interface/display_interface.hpp"

class LVGLDisplayAdapter {
private:
    DisplayInterface* display_;
    lv_disp_drv_t disp_drv_;
    lv_disp_draw_buf_t draw_buf_;
    lv_color_t* buf1_;
    lv_color_t* buf2_;
    uint32_t buf_size_;

    static void flushCallback(lv_disp_drv_t* disp_drv,
                              const lv_area_t* area,
                              lv_color_t* color_p);

public:
    LVGLDisplayAdapter(DisplayInterface* display,
                       lv_color_t* buf1,
                       lv_color_t* buf2,
                       uint32_t buf_size);

    void registerDisplay();
};
