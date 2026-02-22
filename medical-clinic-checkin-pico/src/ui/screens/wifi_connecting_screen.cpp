#include "wifi_connecting_screen.hpp"
#include "lvgl.h"
#include "services/logger_service.hpp"

extern "C" void wifi_connecting_screen_show(const char* msg)
{
    lv_obj_t* scr = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    lv_obj_t* label = lv_label_create(scr);
    lv_label_set_text(label, msg ? msg : "Conectando ao Wi-Fi...");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
    lv_obj_center(label);

    lv_scr_load(scr);

    LOGGER_INFO("[UI] Tela de 'conectando ao Wi-Fi' exibida.");
}
