#include "error_critical_screen.hpp"
#include <lvgl.h>
#include <cstdio>
#include <cstring>

static lv_obj_t *error_screen = nullptr;
static lv_obj_t *title_label = nullptr;
static lv_obj_t *message_label = nullptr;
static lv_obj_t *countdown_label = nullptr;
static lv_obj_t *progress_bar = nullptr;
static int total_seconds = 5;

static char stored_title[128] = "ERRO CRITICO";
static char stored_message[256] = "Erro do sistema";
static int stored_countdown = 5;

static void create_error_critical_screen()
{
    if (error_screen != nullptr)
    {
        lv_obj_clean(error_screen);
    }
    else
    {
        error_screen = lv_obj_create(NULL);
        if (error_screen == NULL)
        {
            printf("[Error Critical Screen] ERROR: Failed to create screen!\n");
            return;
        }
        lv_obj_set_style_bg_color(error_screen, lv_color_hex(0x8B0000), 0);
    }

    total_seconds = stored_countdown;

    title_label = lv_label_create(error_screen);
    if (title_label != NULL)
    {
        lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0);
        lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
        lv_label_set_text(title_label, stored_title);
        lv_obj_align(title_label, LV_ALIGN_CENTER, 0, -60);
    }

    message_label = lv_label_create(error_screen);
    if (message_label != NULL)
    {
        lv_obj_set_style_text_font(message_label, &lv_font_montserrat_18, 0);
        lv_obj_set_style_text_color(message_label, lv_color_hex(0xFFFFFF), 0);
        lv_label_set_text(message_label, stored_message);
        lv_obj_align(message_label, LV_ALIGN_CENTER, 0, -20);
        lv_obj_set_style_text_align(message_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(message_label, 280);
        lv_label_set_long_mode(message_label, LV_LABEL_LONG_WRAP);
    }

    progress_bar = lv_bar_create(error_screen);
    if (progress_bar != NULL)
    {
        lv_obj_set_size(progress_bar, 280, 20);
        lv_obj_align(progress_bar, LV_ALIGN_CENTER, 0, 30);
        lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x440000), LV_PART_MAIN);
        lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0xFF4444), LV_PART_INDICATOR);
        lv_bar_set_range(progress_bar, 0, stored_countdown);
        lv_bar_set_value(progress_bar, stored_countdown, LV_ANIM_OFF);
    }

    countdown_label = lv_label_create(error_screen);
    if (countdown_label != NULL)
    {
        lv_obj_set_style_text_font(countdown_label, &lv_font_montserrat_18, 0);
        lv_obj_set_style_text_color(countdown_label, lv_color_hex(0xFFFFFF), 0);
        char countdown_text[64];
        snprintf(countdown_text, sizeof(countdown_text), "Reiniciando em %d segundos...", stored_countdown);
        lv_label_set_text(countdown_label, countdown_text);
        lv_obj_align(countdown_label, LV_ALIGN_CENTER, 0, 60);
    }
}

void error_critical_screen_show()
{
    if (lv_scr_act() == NULL)
    {
        printf("[Error Critical Screen] ERROR: LVGL not initialized!\n");
        return;
    }

    printf("[Error Critical Screen] Showing: %s - %s\n", stored_title, stored_message);
    create_error_critical_screen();
    
    if (error_screen)
    {
        lv_scr_load(error_screen);
    }
}

void error_critical_screen_update(const char* title, const char* message, int countdown_seconds)
{
    if (title != nullptr) {
        strncpy(stored_title, title, sizeof(stored_title) - 1);
        stored_title[sizeof(stored_title) - 1] = '\0';
    }
    if (message != nullptr) {
        strncpy(stored_message, message, sizeof(stored_message) - 1);
        stored_message[sizeof(stored_message) - 1] = '\0';
    }
    stored_countdown = countdown_seconds;
    total_seconds = countdown_seconds;
    
    if (title_label != nullptr && title != nullptr) {
        lv_label_set_text(title_label, stored_title);
    }
    if (message_label != nullptr && message != nullptr) {
        lv_label_set_text(message_label, stored_message);
    }
    if (progress_bar != nullptr) {
        lv_bar_set_range(progress_bar, 0, countdown_seconds);
        lv_bar_set_value(progress_bar, countdown_seconds, LV_ANIM_OFF);
    }
    if (countdown_label != nullptr) {
        char countdown_text[64];
        snprintf(countdown_text, sizeof(countdown_text), "Reiniciando em %d segundos...", countdown_seconds);
        lv_label_set_text(countdown_label, countdown_text);
    }
}

void error_critical_screen_update_countdown(int seconds_remaining)
{
    if (countdown_label != NULL)
    {
        char countdown_text[64];
        snprintf(countdown_text, sizeof(countdown_text), "Reiniciando em %d segundos...", seconds_remaining);
        lv_label_set_text(countdown_label, countdown_text);
    }
    
    if (progress_bar != NULL)
    {
        lv_bar_set_value(progress_bar, seconds_remaining, LV_ANIM_OFF);
    }
}

void error_critical_screen_clear()
{
    if (error_screen != NULL)
    {
        lv_obj_clean(error_screen);
        lv_obj_set_style_bg_color(error_screen, lv_color_hex(0xFFFFFF), 0);
        lv_obj_invalidate(error_screen);
    }
}