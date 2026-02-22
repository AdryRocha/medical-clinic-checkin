#include "saving_screen.hpp"
#include "services/logger_service.hpp" // Added include
#include <lvgl.h>

static lv_obj_t* scr = nullptr;
static lv_obj_t* spinner = nullptr;
static lv_obj_t* lbl = nullptr;

void saving_screen_show(const char* msg)
{
    if (!scr)
    {
        scr = lv_obj_create(NULL);
        
        spinner = lv_spinner_create(scr, 1000, 60);
        lv_obj_set_size(spinner, 50, 50);
        lv_obj_center(spinner);

        lbl = lv_label_create(scr);
        lv_obj_align_to(lbl, spinner, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    }

    if (msg && *msg) {
        LOGGER_INFO("[SCREEN] Saving: %s", msg); // Fixed macro
        lv_label_set_text(lbl, msg);
    } else {
        lv_label_set_text(lbl, "Processando...");
    }

    lv_scr_load(scr);
}

void saving_screen_show()
{
    saving_screen_show(nullptr);
}