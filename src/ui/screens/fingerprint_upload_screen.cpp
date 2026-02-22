#include "fingerprint_upload_screen.hpp"
#include <stdio.h>

static lv_obj_t* screen = nullptr;
static lv_obj_t* title_label = nullptr;
static lv_obj_t* message_label = nullptr;
static lv_obj_t* spinner = nullptr;

void fingerprint_upload_screen_init() {
    if (screen != nullptr) {
        return;
    }
    
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x065F46), LV_PART_MAIN);
    
    title_label = lv_label_create(screen);
    lv_label_set_text(title_label, "ENVIANDO DIGITAL");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 30);

    spinner = lv_spinner_create(screen, 1000, 60);
    lv_obj_set_size(spinner, 80, 80);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_INDICATOR);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 0);
    
    message_label = lv_label_create(screen);
    lv_label_set_text(message_label, "Sincronizando com servidor...");
    lv_obj_set_style_text_color(message_label, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_obj_set_style_text_font(message_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_align(message_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_width(message_label, 420);
    lv_obj_align(message_label, LV_ALIGN_BOTTOM_MID, 0, -30);
}

void fingerprint_upload_screen_show() {
    if (screen == nullptr) {
        fingerprint_upload_screen_init();
    }
    
    lv_scr_load(screen);
}

void fingerprint_upload_screen_hide() {
}

void fingerprint_upload_screen_update_status(const char* message) {
    if (message_label != nullptr) {
        lv_label_set_text(message_label, message);
    }
}