#include "error_screen.hpp"
#include <lvgl.h>
#include <cstdio>
#include <cstring>

static lv_obj_t* error_screen = nullptr;
static lv_obj_t* icon_container = nullptr;
static lv_obj_t* circle_obj = nullptr;
static lv_obj_t* x_line1 = nullptr;
static lv_obj_t* x_line2 = nullptr;
static lv_obj_t* content_container = nullptr;
static lv_obj_t* title_label = nullptr;
static lv_obj_t* message_label = nullptr;
static lv_obj_t* detail_label = nullptr;

static char stored_title[128] = "Erro";
static char stored_message[256] = "Ocorreu um erro";
static char stored_detail[256] = "";

static void create_error_icon(lv_obj_t* parent, int x_pos, int y_pos)
{
    icon_container = lv_obj_create(parent);
    lv_obj_set_size(icon_container, 60, 60);
    lv_obj_set_style_bg_opa(icon_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(icon_container, 0, 0);
    lv_obj_set_style_pad_all(icon_container, 0, 0);
    lv_obj_clear_flag(icon_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(icon_container, x_pos, y_pos);
    
    circle_obj = lv_obj_create(icon_container);
    lv_obj_set_size(circle_obj, 60, 60);
    lv_obj_set_style_radius(circle_obj, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(circle_obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(circle_obj, lv_color_hex(0xFF9800), 0);
    lv_obj_set_style_border_width(circle_obj, 4, 0);
    lv_obj_center(circle_obj);
    
    static lv_point_t line1_points[2];
    line1_points[0].x = 18;
    line1_points[0].y = 18;
    line1_points[1].x = 42;
    line1_points[1].y = 42;
    
    x_line1 = lv_line_create(icon_container);
    lv_line_set_points(x_line1, line1_points, 2);
    lv_obj_set_style_line_width(x_line1, 4, 0);
    lv_obj_set_style_line_color(x_line1, lv_color_hex(0xFF9800), 0);
    lv_obj_set_style_line_rounded(x_line1, true, 0);
    
    static lv_point_t line2_points[2];
    line2_points[0].x = 42;
    line2_points[0].y = 18;
    line2_points[1].x = 18;
    line2_points[1].y = 42;
    
    x_line2 = lv_line_create(icon_container);
    lv_line_set_points(x_line2, line2_points, 2);
    lv_obj_set_style_line_width(x_line2, 4, 0);
    lv_obj_set_style_line_color(x_line2, lv_color_hex(0xFF9800), 0);
    lv_obj_set_style_line_rounded(x_line2, true, 0);
}

static void create_error_screen()
{
    if (error_screen) {
        lv_obj_clean(error_screen);
    } else {
        error_screen = lv_obj_create(NULL);
        if (!error_screen) {
            printf("[Error Screen] ERROR: Failed to create screen!\n");
            return;
        }
    }
    
    lv_obj_set_style_bg_color(error_screen, lv_color_hex(0xFFF9C4), 0);
    lv_obj_clear_flag(error_screen, LV_OBJ_FLAG_SCROLLABLE);
    
    create_error_icon(error_screen, 25, 130);
    
    content_container = lv_obj_create(error_screen);
    lv_obj_set_size(content_container, 360, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(content_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content_container, 0, 0);
    lv_obj_set_style_pad_all(content_container, 5, 0);
    lv_obj_clear_flag(content_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(content_container, LV_ALIGN_CENTER, 52, 0);
    
    title_label = lv_label_create(content_container);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xF57C00), 0);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(title_label, 350);
    lv_label_set_text(title_label, stored_title);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 0);
    
    message_label = lv_label_create(content_container);
    lv_obj_set_style_text_font(message_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(message_label, lv_color_hex(0x616161), 0);
    lv_obj_set_style_text_align(message_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(message_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(message_label, 350);
    lv_label_set_text(message_label, stored_message);
    lv_obj_align_to(message_label, title_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 18);
    
    if (stored_detail[0] != '\0') {
        detail_label = lv_label_create(content_container);
        lv_obj_set_style_text_font(detail_label, &lv_font_montserrat_18, 0);
        lv_obj_set_style_text_color(detail_label, lv_color_hex(0x9E9E9E), 0);
        lv_obj_set_style_text_align(detail_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_long_mode(detail_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(detail_label, 350);
        lv_label_set_text(detail_label, stored_detail);
        lv_obj_align_to(detail_label, message_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
    }
}

void error_screen_show()
{
    if (lv_scr_act() == NULL) {
        printf("[Error Screen] ERROR: LVGL not initialized!\n");
        return;
    }
    
    printf("[Error Screen] Showing: %s\n", stored_title);
    
    create_error_screen();
    
    if (error_screen) {
        lv_scr_load(error_screen);
    }
}

void error_screen_update(const char* title, const char* message,
                         const char* detail)
{
    if (title) {
        strncpy(stored_title, title, sizeof(stored_title) - 1);
        stored_title[sizeof(stored_title) - 1] = '\0';
    }
    
    if (message) {
        strncpy(stored_message, message, sizeof(stored_message) - 1);
        stored_message[sizeof(stored_message) - 1] = '\0';
    }
    
    if (detail) {
        strncpy(stored_detail, detail, sizeof(stored_detail) - 1);
        stored_detail[sizeof(stored_detail) - 1] = '\0';
    } else {
        stored_detail[0] = '\0';
    }
    
    if (error_screen && title_label && message_label) {
        if (title) {
            lv_label_set_text(title_label, stored_title);
        }
        if (message) {
            lv_label_set_text(message_label, stored_message);
        }
        if (detail_label && detail) {
            lv_label_set_text(detail_label, stored_detail);
        }
    }
}