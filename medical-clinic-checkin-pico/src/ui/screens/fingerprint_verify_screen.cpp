#include "fingerprint_operation_screen.hpp"
#include <stdio.h>

static lv_obj_t* screen = nullptr;
static lv_obj_t* title_label = nullptr;
static lv_obj_t* step_label = nullptr;
static lv_obj_t* icon_container = nullptr;
static lv_obj_t* message_label = nullptr;
static lv_obj_t* status_label = nullptr;
static FingerprintOperationMode current_mode = FINGERPRINT_VERIFY;

static lv_obj_t* make_oval_ring(lv_obj_t* parent, int w, int h, int px, int py,
                                 uint32_t color, int border_w = 2)
{
    lv_obj_t* o = lv_obj_create(parent);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_radius(o, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(o, lv_color_hex(color), 0);
    lv_obj_set_style_border_width(o, border_w, 0);
    lv_obj_set_style_border_opa(o, LV_OPA_COVER, 0);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(o, px, py);
    return o;
}

static void create_fingerprint_icon(lv_obj_t* parent, int x, int y, uint32_t color)
{
    icon_container = lv_obj_create(parent);
    lv_obj_set_size(icon_container, 80, 100);
    lv_obj_set_style_bg_opa(icon_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(icon_container, 0, 0);
    lv_obj_set_style_pad_all(icon_container, 0, 0);
    lv_obj_clear_flag(icon_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(icon_container, x, y);

    make_oval_ring(icon_container, 76, 96,  2,  2,  color);
    make_oval_ring(icon_container, 64, 82,  8,  9,  color);
    make_oval_ring(icon_container, 52, 68,  14, 16, color);
    make_oval_ring(icon_container, 40, 54,  20, 23, color);
    make_oval_ring(icon_container, 28, 40,  26, 30, color);
    make_oval_ring(icon_container, 16, 26,  32, 37, color);

    lv_obj_t* dot = lv_obj_create(icon_container);
    lv_obj_set_size(dot, 6, 6);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dot, lv_color_hex(color), 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_set_pos(dot, 37, 47);
}

void fingerprint_operation_screen_init()
{
    if (screen != nullptr) {
        return;
    }
    
    screen = lv_obj_create(NULL);
}

void fingerprint_operation_screen_show(FingerprintOperationMode mode)
{
    if (screen == nullptr) {
        fingerprint_operation_screen_init();
    }
    
    lv_obj_clean(screen);
    
    current_mode = mode;
    
    if (mode == FINGERPRINT_VERIFY) {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x1E3A8A), LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x065F46), LV_PART_MAIN);
    }
    
    title_label = lv_label_create(screen);
    if (mode == FINGERPRINT_VERIFY) {
        lv_label_set_text(title_label, "VERIFICACAO BIOMETRICA");
    } else {
        lv_label_set_text(title_label, "CADASTRO DE DIGITAL");
    }
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 30);
    
    if (mode == FINGERPRINT_ENROLL) {
        step_label = lv_label_create(screen);
        lv_label_set_text(step_label, "Etapa 1 de 2");
        lv_obj_set_style_text_color(step_label, lv_color_hex(0x6EE7B7), LV_PART_MAIN);
        lv_obj_set_style_text_font(step_label, &lv_font_montserrat_18, LV_PART_MAIN);
        lv_obj_align(step_label, LV_ALIGN_TOP_MID, 0, 60);
    }
    
    uint32_t icon_color = (mode == FINGERPRINT_VERIFY) ? 0x60A5FA : 0x34D399;
    create_fingerprint_icon(screen, 200, 95, icon_color);
    
    message_label = lv_label_create(screen);
    if (mode == FINGERPRINT_VERIFY) {
        lv_label_set_text(message_label, "Posicione seu dedo no sensor");
    } else {
        lv_label_set_text(message_label, "Posicione seu dedo no sensor\ne mantenha pressionado");
    }
    lv_obj_set_style_text_color(message_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(message_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_align(message_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_width(message_label, 420);
    lv_obj_align(message_label, LV_ALIGN_CENTER, 0, 80);
    
    status_label = lv_label_create(screen);
    lv_label_set_text(status_label, "Aguardando digital...");
    uint32_t status_color = (mode == FINGERPRINT_VERIFY) ? 0x93C5FD : 0xA7F3D0;
    lv_obj_set_style_text_color(status_label, lv_color_hex(status_color), LV_PART_MAIN);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_align(status_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_width(status_label, 420);
    lv_obj_align(status_label, LV_ALIGN_BOTTOM_MID, 0, -30);
    
    lv_scr_load(screen);
    printf("[FingerprintOperationScreen] Shown - Mode: %s\n", 
           mode == FINGERPRINT_VERIFY ? "VERIFY" : "ENROLL");
}

void fingerprint_operation_screen_hide()
{
}

void fingerprint_operation_screen_set_step(int step)
{
    if (current_mode != FINGERPRINT_ENROLL || step_label == nullptr) {
        return;
    }
    
    if (step == 1) {
        lv_label_set_text(step_label, "Etapa 1 de 2");
        lv_label_set_text(message_label, "Posicione seu dedo no sensor\ne mantenha pressionado");
    } else if (step == 2) {
        lv_label_set_text(step_label, "Etapa 1 de 2 - Concluida!");
        lv_label_set_text(message_label, "Levante o dedo do sensor");
    } else if (step == 3) {
        lv_label_set_text(step_label, "Etapa 2 de 2");
        lv_label_set_text(message_label, "Posicione o mesmo dedo\nnovamente no sensor");
    }
}

void fingerprint_operation_screen_update_status(const char* message)
{
    if (status_label != nullptr) {
        lv_label_set_text(status_label, message);
    }
}