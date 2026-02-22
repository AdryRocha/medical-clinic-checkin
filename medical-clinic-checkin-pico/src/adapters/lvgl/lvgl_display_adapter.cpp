#include "adapters/lvgl/lvgl_display_adapter.hpp"
#include "services/logger_service.hpp"
#include <stdio.h>
#include "../../drivers/display/interface/display_interface.hpp"

// --- CONSTRUTOR ---
LVGLDisplayAdapter::LVGLDisplayAdapter(DisplayInterface* driver, uint8_t* buffer, uint8_t* buffer2, size_t buf_size_px)
    : driver_(driver), disp_handle_(nullptr)
{
    // 1. Inicializa o buffer de desenho do LVGL
    // buffer2 pode ser nullptr se você não quiser double buffering
    lv_disp_draw_buf_init(&disp_buf_, buffer, buffer2, buf_size_px);
}

// --- CALLBACK DE FLUSH (ESTÁTICO) ---
void LVGLDisplayAdapter::flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
    // Recupera o ponteiro da instância através do user_data
    auto* adapter = static_cast<LVGLDisplayAdapter*>(disp_drv->user_data);

    if (adapter && adapter->driver_) {
        // Chama o método do driver de hardware
        // Nota: O cast para (const uint16_t*) assume cor de 16 bits (RGB565)
        adapter->driver_->drawPixels(area->x1, area->y1,
                                     area->x2 - area->x1 + 1,
                                     area->y2 - area->y1 + 1,
                                     (const uint16_t*)&color_p->full);
    }

    // Informa ao LVGL que o desenho terminou
    lv_disp_flush_ready(disp_drv);
}

// --- REGISTRO DO DISPLAY ---
void LVGLDisplayAdapter::registerDisplay() {
    Logger::info("[ADAPTER] Registrando Display LVGL...");

    // 1. Inicializa a estrutura do driver do display
    lv_disp_drv_init(&disp_drv_);

    // 2. Configurações essenciais
    disp_drv_.hor_res = 480;       // Ajuste conforme seu display
    disp_drv_.ver_res = 320;       // Ajuste conforme seu display
    disp_drv_.flush_cb = flush_cb; // Aponta para a função estática acima
    disp_drv_.draw_buf = &disp_buf_; // Aponta para o buffer iniciado no construtor
    
    // 3. Ponteiro User Data (CRÍTICO para o flush_cb funcionar)
    disp_drv_.user_data = this;

    // 4. Registra no LVGL
    disp_handle_ = lv_disp_drv_register(&disp_drv_);

    // 5. Verificação de sucesso
    if (!disp_handle_) {
        Logger::error("[ADAPTER] Falha ao registrar display!");
    } else {
        Logger::info("[ADAPTER] Display Registrado com Sucesso.");
    }
} // <--- A função termina AQUI. O erro anterior era fechar essa chave antes do 'if'.