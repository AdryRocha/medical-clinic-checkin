#include "appointment_screen.hpp"
#include "services/time_service.hpp"
#include <lvgl.h>
#include <stdio.h>
#include <string.h>

static lv_obj_t* scr = nullptr;
static lv_obj_t* label_title = nullptr;
static lv_obj_t* label_patient = nullptr;
static lv_obj_t* label_cpf = nullptr;
static lv_obj_t* label_doctor = nullptr;
static lv_obj_t* label_specialty = nullptr;
static lv_obj_t* label_date_time = nullptr;
static lv_obj_t* label_instructions = nullptr;
static lv_obj_t* success_icon = nullptr;

static char stored_patient[128] = "N/A";
static char stored_cpf[128] = "N/A";
static char stored_appt[128] = "N/A";
static char stored_professional_info[128] = "N/A";

static void create_success_icon(lv_obj_t* parent, int x, int y)
{
    success_icon = lv_obj_create(parent);
    lv_obj_set_size(success_icon, 50, 50);
    lv_obj_set_pos(success_icon, x, y);
    lv_obj_set_style_bg_color(success_icon, lv_color_hex(0x4CAF50), 0);
    lv_obj_set_style_radius(success_icon, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(success_icon, 0, 0);
    lv_obj_set_style_pad_all(success_icon, 0, 0);
    lv_obj_clear_flag(success_icon, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* symbol = lv_label_create(success_icon);
    lv_label_set_text(symbol, LV_SYMBOL_OK);
    lv_obj_set_style_text_color(symbol, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(symbol, &lv_font_montserrat_18, 0);
    lv_obj_center(symbol);
}

static void create_appointment_screen()
{
    if (scr) {
        lv_obj_clean(scr);
    } else {
        scr = lv_obj_create(NULL);
        lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(scr, LV_HOR_RES, LV_VER_RES);
    }
    
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xC8E6C9), 0);

    create_success_icon(scr, 215, 30);

    label_title = lv_label_create(scr);
    lv_obj_set_style_text_color(label_title, lv_color_hex(0x2E7D32), 0);
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 90);

    label_patient = lv_label_create(scr);
    lv_obj_set_style_text_color(label_patient, lv_color_hex(0x1B5E20), 0);
    lv_label_set_long_mode(label_patient, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label_patient, LV_HOR_RES - 40);
    lv_obj_align(label_patient, LV_ALIGN_TOP_LEFT, 20, 130);

    label_cpf = lv_label_create(scr);
    lv_obj_set_style_text_color(label_cpf, lv_color_hex(0x616161), 0);
    lv_obj_align(label_cpf, LV_ALIGN_TOP_LEFT, 20, 160);

    label_doctor = lv_label_create(scr);
    lv_obj_set_style_text_color(label_doctor, lv_color_hex(0x1B5E20), 0);
    lv_label_set_long_mode(label_doctor, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label_doctor, LV_HOR_RES - 40);
    lv_obj_align(label_doctor, LV_ALIGN_TOP_LEFT, 20, 190);

    label_specialty = lv_label_create(scr);
    lv_obj_set_style_text_color(label_specialty, lv_color_hex(0x1B5E20), 0);
    lv_obj_align(label_specialty, LV_ALIGN_TOP_LEFT, 20, 220);

    label_date_time = lv_label_create(scr);
    lv_obj_set_style_text_color(label_date_time, lv_color_hex(0x1B5E20), 0);
    lv_label_set_long_mode(label_date_time, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label_date_time, LV_HOR_RES - 40);
    lv_obj_align(label_date_time, LV_ALIGN_TOP_LEFT, 20, 250);

    label_instructions = lv_label_create(scr);
    lv_obj_set_style_text_color(label_instructions, lv_color_hex(0x2E7D32), 0);
    lv_obj_align(label_instructions, LV_ALIGN_BOTTOM_MID, 0, -20);
}

void appointment_screen_show()
{
    create_appointment_screen();
    appointment_screen_update(stored_patient, stored_cpf, stored_appt, stored_professional_info);
    lv_scr_load(scr);
}

void appointment_screen_update(const char* patient_name, 
                            const char* cpf,
                            const char* appointment_id,
                            const char* professional_info)
{
    snprintf(stored_patient, sizeof(stored_patient), "%s", 
             patient_name ? patient_name : "N/A");
    snprintf(stored_cpf, sizeof(stored_cpf), "%s", 
             cpf ? cpf : "N/A");
    snprintf(stored_appt, sizeof(stored_appt), "%s", 
             appointment_id ? appointment_id : "N/A");
    snprintf(stored_professional_info, sizeof(stored_professional_info), "%s",
             professional_info ? professional_info : "N/A");
    
    if (!scr || !label_patient || !label_cpf || !label_doctor || !label_specialty || 
        !label_date_time || !label_title || !label_instructions) {
        return;
    }
    
    lv_label_set_text(label_title, "Check-in Confirmado!");
    
    char patient_text[140];
    snprintf(patient_text, sizeof(patient_text), "Paciente: %s", stored_patient);
    lv_label_set_text(label_patient, patient_text);
    
    char cpf_masked[32];
    int cpf_len = strlen(stored_cpf);
    if (cpf_len >= 3) {
        snprintf(cpf_masked, sizeof(cpf_masked), "CPF: %c%c%c.***.***-**", 
                 stored_cpf[0], stored_cpf[1], stored_cpf[2]);
    } else {
        snprintf(cpf_masked, sizeof(cpf_masked), "CPF: %s", stored_cpf);
    }
    lv_label_set_text(label_cpf, cpf_masked);
    
    TimeService& time_service = TimeService::getInstance();
    std::string date_str = time_service.getDateString();
    
    char date_time_text[100];
    snprintf(date_time_text, sizeof(date_time_text), "Data: %s - %s", 
             date_str.c_str(), stored_appt);
    lv_label_set_text(label_date_time, date_time_text);
    
    if (strcmp(stored_professional_info, "N/A") != 0) {
        char doctor_text[100];
        char specialty_text[100];
        
        const char* separator = strstr(stored_professional_info, " - ");
        if (separator) {
            int name_len = separator - stored_professional_info;
            snprintf(doctor_text, sizeof(doctor_text), "Medico: %.*s", name_len, stored_professional_info);
            snprintf(specialty_text, sizeof(specialty_text), "Especialidade: %s", separator + 3);
        } else {
            snprintf(doctor_text, sizeof(doctor_text), "Medico: %s", stored_professional_info);
            snprintf(specialty_text, sizeof(specialty_text), "Especialidade: N/A");
        }
        
        lv_label_set_text(label_doctor, doctor_text);
        lv_label_set_text(label_specialty, specialty_text);
    } else {
        lv_label_set_text(label_doctor, "Medico: N/A");
        lv_label_set_text(label_specialty, "Especialidade: N/A");
    }
    
    lv_label_set_text(label_instructions, "Aguarde ser chamado");
}