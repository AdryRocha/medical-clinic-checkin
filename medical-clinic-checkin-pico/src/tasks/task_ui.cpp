#include "tasks/task_ui.hpp"
#include "services/logger_service.hpp"
#include "core/checkin_state_machine.hpp"
#include "lvgl.h"

#include "ui/screens/welcome_screen.hpp"
#include "ui/screens/qrcode_screen.hpp"
#include "ui/screens/fingerprint_verify_screen.hpp"
#include "ui/screens/confirmation_screen.hpp"
#include "ui/screens/error_screen.hpp"

extern SemaphoreHandle_t g_lvgl_mutex;
// Força renderização inicial: começa com valor sentinela diferente do estado atual
static CheckinState last_state = static_cast<CheckinState>(-1);

// --- ESTILOS VISUAIS ---
// Estilos primários para botões principais (azul corporativo)
static lv_style_t style_btn_primary;
// Estilos secundários para ações destrutivas (vermelho)
static lv_style_t style_btn_secondary;
// Feedback visual de pressão (zoom)
static lv_style_t style_btn_pressed;
static bool styles_initialized = false;

static void init_styles() {
    if (styles_initialized) return;

    // Estilo primário: azul corporativo com gradiente
    lv_style_init(&style_btn_primary);
    lv_style_set_radius(&style_btn_primary, 10);
    lv_style_set_bg_opa(&style_btn_primary, LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn_primary, lv_color_hex(0x0055A4));
    lv_style_set_bg_grad_color(&style_btn_primary, lv_color_hex(0x004080));
    lv_style_set_bg_grad_dir(&style_btn_primary, LV_GRAD_DIR_VER);
    lv_style_set_text_color(&style_btn_primary, lv_color_hex(0xFFFFFF));
    lv_style_set_shadow_width(&style_btn_primary, 15);
    lv_style_set_shadow_ofs_y(&style_btn_primary, 5);

    // Estilo secundário: vermelho para cancelar/rejeitar
    lv_style_init(&style_btn_secondary);
    lv_style_set_radius(&style_btn_secondary, 10);
    lv_style_set_bg_opa(&style_btn_secondary, LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn_secondary, lv_color_hex(0xCC3333));
    lv_style_set_text_color(&style_btn_secondary, lv_color_hex(0xFFFFFF));
    lv_style_set_shadow_width(&style_btn_secondary, 15);
    lv_style_set_shadow_ofs_y(&style_btn_secondary, 5);

    // Feedback de pressão: zoom 110%
    lv_style_init(&style_btn_pressed);
    lv_style_set_transform_zoom(&style_btn_pressed, 280); // 280/256 = ~110%
    
    styles_initialized = true;
    Logger::info("[UI] Estilos visuais inicializados (Primary, Secondary, Pressed).");
}

TaskUI::TaskUI(DisplayInterface* display, TouchInterface* touch)
    : display_(display), touch_(touch) {}

void TaskUI::taskEntry(void* arg) {
    static_cast<TaskUI*>(arg)->run();
}

void TaskUI::run() {
    // Criar mutexes APÓS scheduler iniciar (evita travamento)
    extern SemaphoreHandle_t g_sm_mutex;
    if (!g_lvgl_mutex) {
        g_lvgl_mutex = xSemaphoreCreateMutex();
        Logger::info("[UI] g_lvgl_mutex criado.");
    }
    if (!g_sm_mutex) {
        g_sm_mutex = xSemaphoreCreateMutex();
        Logger::info("[UI] g_sm_mutex criado.");
    }
    
    init_styles();
    Logger::info("[UI] Task UI iniciada - aguardando primeiro ciclo...");
    
    while (true) {
        // Tenta pegar o mutex com timeout para não travar a task
        if (g_lvgl_mutex && xSemaphoreTake(g_lvgl_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            
            static uint32_t last_heartbeat = 0;
            if (lv_tick_get() - last_heartbeat > 1000) {
                Logger::info("[UI] Heartbeat - LVGL Tick: %u", lv_tick_get());
                last_heartbeat = lv_tick_get();
            }
                // Loga estado da task UI e heap livre
                Logger::info("[UI] TaskUI loop: heap=%u", xPortGetFreeHeapSize());
            // ------------------------------

            CheckinState current = CheckinStateMachine::instance().get_state();
            if (current != last_state) {
                switch(current) {
                    case CheckinState::AGUARDANDO_QR: welcome_screen_show(); break;
                    case CheckinState::PROCESSANDO_DADOS_PACIENTE: qrcode_screen_show("Validando..."); break;
                    case CheckinState::VALIDACAO_BIOMETRICA: fingerprint_verify_screen_show("Aproxime o dedo"); break;
                    case CheckinState::CONSULTA_CONFIRMADA: confirmation_screen_show("Sucesso!"); break;
                    case CheckinState::EM_ERRO: error_screen_show("Falha"); break;
                    default: break;
                }
                last_state = current;
            }

            lv_tick_inc(10);
            uint32_t delay_ms = lv_timer_handler(); // Processa a renderização
            xSemaphoreGive(g_lvgl_mutex);

            // Hedging contra valores inválidos de delay
            if (delay_ms == 0xFFFFFFFF || delay_ms == 0) {
                vTaskDelay(pdMS_TO_TICKS(10));
            } else {
                if (delay_ms > 50) delay_ms = 50; // Cap de delay para manter responsividade
                vTaskDelay(pdMS_TO_TICKS(delay_ms));
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}