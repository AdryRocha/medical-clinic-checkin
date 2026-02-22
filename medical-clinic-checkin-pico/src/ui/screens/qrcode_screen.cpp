#include "qrcode_screen.hpp"
#include "services/logger_service.hpp" 
#include <lvgl.h>

static lv_obj_t* scr = nullptr;
static lv_obj_t* label_status = nullptr;

// --- NOVA FUNÇÃO: Resolve o erro de undefined reference ---
// Essa implementação estava faltando para atender ao .hpp
void qrcode_screen_show()
{
    // Chama a versão principal passando nullptr ou string vazia
    qrcode_screen_show(nullptr);
}

// Função Principal
void qrcode_screen_show(const char* msg)
{
    if (!scr)
    {
        scr = lv_obj_create(NULL);
        lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
        
        // Ícone ou texto simulando ícone
        lv_obj_t* icon = lv_label_create(scr);
        lv_label_set_text(icon, "[ QR ]");
        lv_obj_align(icon, LV_ALIGN_CENTER, 0, -20);

        // Label de status dinâmico
        label_status = lv_label_create(scr);
        lv_obj_align(label_status, LV_ALIGN_CENTER, 0, 20);
    }

    if (msg && *msg) {
        LOGGER_INFO("[SCREEN] QR: %s", msg); 
        lv_label_set_text(label_status, msg);
    } else {
        lv_label_set_text(label_status, "Lendo QR Code...");
    }

    lv_scr_load(scr);
}