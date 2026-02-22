#include "init_status_screen.hpp"
#include <lvgl.h>
#include <cstdio>

static lv_obj_t *init_status_screen = nullptr;
static lv_obj_t *title_label = nullptr;
static lv_obj_t *message_label = nullptr;
static lv_obj_t *spinner = nullptr;

static void create_init_status_screen(const char *title, const char *message)
{
    if (init_status_screen != nullptr)
    {
        lv_obj_clean(init_status_screen);
    }
    else
    {
        init_status_screen = lv_obj_create(NULL);
        if (init_status_screen == NULL)
        {
            printf("[Init Status Screen] ERROR: Failed to create screen!\n");
            return;
        }
        lv_obj_set_style_bg_color(init_status_screen, lv_color_hex(0x000000), 0);
    }

    title_label = lv_label_create(init_status_screen);
    if (title_label == NULL)
        return;
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(title_label, title);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, -60);

    spinner = lv_spinner_create(init_status_screen, 1000, 60);
    if (spinner == NULL)
        return;
    lv_obj_set_size(spinner, 50, 50);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0x00A8E8), LV_PART_INDICATOR);

    message_label = lv_label_create(init_status_screen);
    if (message_label == NULL)
        return;
    lv_obj_set_style_text_font(message_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(message_label, lv_color_hex(0xAAAAAA), 0);
    lv_label_set_text(message_label, message);
    lv_obj_align(message_label, LV_ALIGN_CENTER, 0, 60);
    lv_obj_set_style_text_align(message_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(message_label, 280);
}

void init_status_screen_show(const char *title, const char *message)
{
    if (lv_scr_act() == NULL)
    {
        printf("[Init Status Screen] ERROR: LVGL not initialized!\n");
        return;
    }

    printf("[Init Status Screen] %s\n", title);
    create_init_status_screen(title, message);
    
    if (init_status_screen)
    {
        lv_scr_load(init_status_screen);
    }
}

void init_status_screen_update(const char *title, const char *message)
{
    if (title != nullptr && title_label != nullptr)
    {
        lv_label_set_text(title_label, title);
    }
    if (message != nullptr && message_label != nullptr)
    {
        lv_label_set_text(message_label, message);
    }
}