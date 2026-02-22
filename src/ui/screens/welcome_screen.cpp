#include "welcome_screen.hpp"
#include "services/time_service.hpp"
#include <lvgl.h>

static lv_obj_t* scr = nullptr;
static lv_obj_t* label_title = nullptr;
static lv_obj_t* label_message = nullptr;
static lv_obj_t* label_time = nullptr;
static lv_obj_t* label_date = nullptr;
static lv_obj_t* qr_icon = nullptr;

static void create_welcome_screen()
{
    if (scr) {
        lv_obj_clean(scr);
    } else {
        scr = lv_obj_create(NULL);
    }
    
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(scr, LV_HOR_RES, LV_VER_RES);

    label_time = lv_label_create(scr);
    lv_label_set_text(label_time, "00:00:00");
    lv_obj_align(label_time, LV_ALIGN_TOP_RIGHT, -10, 10);
    
    label_date = lv_label_create(scr);
    lv_label_set_text(label_date, "00/00/0000");
    lv_obj_align(label_date, LV_ALIGN_TOP_LEFT, 10, 10);

    label_title = lv_label_create(scr);
    lv_label_set_text(label_title, "Bem-vindo(a)!");
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 50);

    // Ícone QR Code simplificado (apenas ilustrativo)
    qr_icon = lv_obj_create(scr);
    lv_obj_set_size(qr_icon, 80, 80);
    lv_obj_align(qr_icon, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_style_bg_color(qr_icon, lv_color_white(), 0);
    lv_obj_set_style_border_color(qr_icon, lv_color_make(200, 200, 200), 0);
    lv_obj_set_style_border_width(qr_icon, 1, 0);
    lv_obj_set_style_radius(qr_icon, 0, 0);
    lv_obj_set_style_pad_all(qr_icon, 0, 0); // Remover padding padrão do LVGL
    lv_obj_clear_flag(qr_icon, LV_OBJ_FLAG_SCROLLABLE);

    // Helper para criar os "olhos" do QR Code (Finder Patterns)
    auto create_finder = [](lv_obj_t* parent, int x, int y, int size) {
        // Quadrado externo preto
        lv_obj_t* outer = lv_obj_create(parent);
        lv_obj_set_size(outer, size, size);
        lv_obj_set_pos(outer, x, y);
        lv_obj_set_style_bg_color(outer, lv_color_black(), 0);
        lv_obj_set_style_border_width(outer, 0, 0);
        lv_obj_set_style_radius(outer, 0, 0);
        lv_obj_clear_flag(outer, LV_OBJ_FLAG_SCROLLABLE);

        // Quadrado interno branco
        lv_obj_t* inner = lv_obj_create(outer);
        lv_obj_set_size(inner, size - 8, size - 8);
        lv_obj_align(inner, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(inner, lv_color_white(), 0);
        lv_obj_set_style_border_width(inner, 0, 0);
        lv_obj_set_style_radius(inner, 0, 0);
        lv_obj_clear_flag(inner, LV_OBJ_FLAG_SCROLLABLE);

        // Quadrado central preto
        lv_obj_t* center = lv_obj_create(inner);
        lv_obj_set_size(center, size - 14, size - 14);
        lv_obj_align(center, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(center, lv_color_black(), 0);
        lv_obj_set_style_border_width(center, 0, 0);
        lv_obj_set_style_radius(center, 0, 0);
        lv_obj_clear_flag(center, LV_OBJ_FLAG_SCROLLABLE);
    };

    // Criar os 3 finders
    create_finder(qr_icon, 5, 5, 24);   // Topo Esquerdo
    create_finder(qr_icon, 51, 5, 24);  // Topo Direito
    create_finder(qr_icon, 5, 51, 24);  // Base Esquerda

    // Criar alguns "dados" aleatórios simples no meio (quadradinhos pretos)
    // Para ser leve, criar apenas alguns representativos
    auto create_dot = [](lv_obj_t* parent, int x, int y, int w, int h) {
        lv_obj_t* dot = lv_obj_create(parent);
        lv_obj_set_size(dot, w, h);
        lv_obj_set_pos(dot, x, y);
        lv_obj_set_style_bg_color(dot, lv_color_black(), 0);
        lv_obj_set_style_border_width(dot, 0, 0);
        lv_obj_set_style_radius(dot, 0, 0);
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);
    };

    // Pontos simulando dados
    create_dot(qr_icon, 34, 10, 12, 6);
    create_dot(qr_icon, 34, 20, 6, 12);
    create_dot(qr_icon, 44, 24, 6, 6);
    create_dot(qr_icon, 10, 34, 12, 6);
    create_dot(qr_icon, 26, 34, 6, 12);
    create_dot(qr_icon, 36, 36, 12, 12); // Bloco central
    create_dot(qr_icon, 54, 34, 12, 6);
    create_dot(qr_icon, 60, 44, 6, 6);
    create_dot(qr_icon, 34, 52, 6, 6);
    create_dot(qr_icon, 44, 52, 12, 12);
    create_dot(qr_icon, 60, 60, 10, 10); // Canto inferior direito (dados)

    label_message = lv_label_create(scr);
    lv_label_set_long_mode(label_message, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label_message, lv_pct(90));
    lv_obj_set_style_text_align(label_message, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_message, LV_ALIGN_CENTER, 0, 60);
}

void welcome_screen_show(const char* msg)
{
    create_welcome_screen();
    
    if (msg && *msg) {
        lv_label_set_text(label_message, msg);
    } else {
        lv_label_set_text(label_message, "Aproxime o QR Code do leitor para iniciar o check-in.");
    }
    
    welcome_screen_update_time();
    lv_scr_load(scr);
}

void welcome_screen_update(const char* msg)
{
    if (!scr || !label_message) {
        return;
    }
    
    if (msg && *msg) {
        lv_label_set_text(label_message, msg);
    } else {
        lv_label_set_text(label_message, "Aproxime o QR Code do leitor para iniciar o check-in.");
    }
}

void welcome_screen_update_time()
{
    if (!scr || !label_time || !label_date) {
        return;
    }
    
    TimeService& time_service = TimeService::getInstance();
    
    std::string time_str = time_service.getTimeString();
    std::string date_str = time_service.getDateString();
    
    lv_label_set_text(label_time, time_str.c_str());
    lv_label_set_text(label_date, date_str.c_str());
}