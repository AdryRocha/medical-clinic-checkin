#include "ui/screens/ui_wifi_icon.hpp" 
#include "lvgl.h"
#include "FreeRTOS.h"
#include "semphr.h"

// Mutex global do LVGL (definido no main_embedded.cpp)
extern SemaphoreHandle_t g_lvgl_mutex;

// ============================================================
// OBJETOS ESTÁTICOS
// ============================================================

static lv_obj_t* wifi_icon_label = nullptr;

// ============================================================
// API PÚBLICA
// ============================================================

extern "C" void ui_create_wifi_icon()
{
    if (!g_lvgl_mutex) return;

    if (xSemaphoreTake(g_lvgl_mutex, portMAX_DELAY)) {

        if (wifi_icon_label == nullptr) {
            // Usa lv_layer_top() para ficar visível sobre todas as telas
            wifi_icon_label = lv_label_create(lv_layer_top());
            
            // Estado inicial: WiFi cortado
            lv_label_set_text(wifi_icon_label, LV_SYMBOL_WIFI);
            lv_obj_set_style_text_color(wifi_icon_label, lv_palette_main(LV_PALETTE_GREY), 0);
            
            // Tenta usar fonte maior se disponível
            #if LV_FONT_MONTSERRAT_18
                lv_obj_set_style_text_font(wifi_icon_label, &lv_font_montserrat_18, 0);
            #endif

            // Canto superior direito
            lv_obj_align(wifi_icon_label, LV_ALIGN_TOP_RIGHT, -10, 5);
        }

        xSemaphoreGive(g_lvgl_mutex);
    }
}

extern "C" void ui_set_wifi_status(WiFiStatus status)
{
    if (!wifi_icon_label || !g_lvgl_mutex) return;

    if (xSemaphoreTake(g_lvgl_mutex, portMAX_DELAY)) {

        switch (status) {
            case WiFiStatus::CONNECTED:
                lv_label_set_text(wifi_icon_label, LV_SYMBOL_WIFI);
                lv_obj_set_style_text_color(wifi_icon_label, lv_palette_main(LV_PALETTE_GREEN), 0);
                break;

            case WiFiStatus::CONNECTING:
                lv_label_set_text(wifi_icon_label, LV_SYMBOL_REFRESH);
                lv_obj_set_style_text_color(wifi_icon_label, lv_palette_main(LV_PALETTE_ORANGE), 0);
                break;

            case WiFiStatus::DISCONNECTED:
            case WiFiStatus::ERROR:
            default:
                lv_label_set_text(wifi_icon_label, LV_SYMBOL_WARNING); 
                lv_obj_set_style_text_color(wifi_icon_label, lv_palette_main(LV_PALETTE_RED), 0);
                break;
        }

        xSemaphoreGive(g_lvgl_mutex);
    }
}