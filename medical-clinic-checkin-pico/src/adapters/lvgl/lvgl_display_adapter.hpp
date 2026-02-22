#pragma once

#include "lvgl.h"
#include "drivers/display/interface/display_interface.hpp"

class LVGLDisplayAdapter {
public:
    // Construtor: Aceita o driver, o buffer de pixels e o tamanho do buffer (em pixels)
    LVGLDisplayAdapter(DisplayInterface* driver, uint8_t* buffer, uint8_t* buffer2, size_t buf_size_px);

    // Registra o display no LVGL
    void registerDisplay();

private:
    // Callback estático exigido pelo LVGL
    static void flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

    DisplayInterface* driver_;
    
    // Estruturas internas do LVGL
    lv_disp_draw_buf_t disp_buf_; 
    lv_disp_drv_t disp_drv_; 
    lv_color_t* buf1_;  // estes são os buffers de desenho (double buffering opcional)
    lv_color_t* buf2_;  // este são os buffers de desenho (double buffering opcional)
    uint32_t buf_size_;  // estes armazenam o tamanho do buffer em pixels (não bytes)   
    lv_disp_t* disp_handle_;  // estes armazenam o handle do display registrado no LVGL
};