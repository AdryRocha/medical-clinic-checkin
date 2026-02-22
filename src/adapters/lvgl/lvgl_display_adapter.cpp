#include "lvgl_display_adapter.hpp"
#include <stdio.h>

#include "../../drivers/display/interface/display_interface.hpp"

LVGLDisplayAdapter::LVGLDisplayAdapter(DisplayInterface* display,
                                       lv_color_t* buf1,
                                       lv_color_t* buf2,
                                       uint32_t buf_size)
    : display_(display), buf1_(buf1), buf2_(buf2) {
    
    // Inicializa o buffer de desenho do LVGL
    lv_disp_draw_buf_init(&draw_buf_, buf1_, buf2_, buf_size);
    
    // Inicializa o driver de display do LVGL
    lv_disp_drv_init(&disp_drv_);
    
    // Configura resolução baseada no hardware
    disp_drv_.hor_res = display_->getWidth();
    disp_drv_.ver_res = display_->getHeight();
    
    // Configura callback de flush
    disp_drv_.flush_cb = flushCallback;
    
    // Vincula buffer e dados do usuário
    disp_drv_.draw_buf = &draw_buf_;
    disp_drv_.user_data = this;
}

void LVGLDisplayAdapter::flushCallback(lv_disp_drv_t* disp_drv, 
                                       const lv_area_t* area, 
                                       lv_color_t* color_p) {
    LVGLDisplayAdapter* adapter = (LVGLDisplayAdapter*)disp_drv->user_data;
    
    if (adapter && adapter->display_) {
        // Chama o driver de hardware para desenhar a área
        adapter->display_->drawPixels(
            area->x1, area->y1, 
            area->x2, area->y2,
            (const uint16_t*)color_p
        );
    }
    
    // Avisa o LVGL que o desenho terminou
    lv_disp_flush_ready(disp_drv);
}

void LVGLDisplayAdapter::registerDisplay() {
    // Registra o driver no LVGL
    lv_disp_drv_register(&disp_drv_);
}