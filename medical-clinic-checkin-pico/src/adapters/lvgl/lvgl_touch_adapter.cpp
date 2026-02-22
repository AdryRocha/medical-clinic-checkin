#include "adapters/lvgl/lvgl_touch_adapter.hpp"
#include "services/logger_service.hpp"
#include "drivers/touch/ft6336u/ft6336u_driver.hpp" 

#include <algorithm>

static LVGLTouchAdapter* instance = nullptr;

LVGLTouchAdapter::LVGLTouchAdapter(TouchInterface* touch, uint16_t screen_width, uint16_t screen_height)
    : touch_(touch), width_(screen_width), height_(screen_height) {}

void LVGLTouchAdapter::registerInputDevice() {
    lv_indev_drv_init(&indev_drv_);
    indev_drv_.type = LV_INDEV_TYPE_POINTER;
    indev_drv_.user_data = this; 
    indev_drv_.read_cb = read_cb;
    
    lv_indev_t* indev = lv_indev_drv_register(&indev_drv_);
    if (!indev) {
        Logger::error("[ADAPTER] Falha ao registrar touch no LVGL.");
        return;
    }
    Logger::info("[ADAPTER] Touch registrado no LVGL.");
}

void LVGLTouchAdapter::read_cb(lv_indev_drv_t* drv, lv_indev_data_t* data) {
    auto* self = static_cast<LVGLTouchAdapter*>(drv->user_data);
    TouchPoint p;

    // Evita spam no log: só loga na transição press/release
    static bool last_pressed = false;
    static uint32_t last_poll_log_ms = 0;
    static uint16_t last_x = 0;
    static uint16_t last_y = 0;

    // Confirma que o LVGL está chamando o driver (log 1x por segundo)
    const uint32_t now_ms = lv_tick_get();
    if ((now_ms - last_poll_log_ms) >= 1000u) {
        Logger::info("[TOUCH] indev poll alive");
        last_poll_log_ms = now_ms;
    }

        // Loga coordenadas e estado do touch a cada polling
        Logger::info("[TOUCH] read_cb: valid=%d, x=%u, y=%u", p.valid, p.x, p.y);

    if (self->touch_->readPoint(&p) && p.valid) {
        const uint16_t w = self->width_;
        const uint16_t h = self->height_;

        uint16_t x = 0;
        uint16_t y = 0;

        // Escala coordenadas 12-bit (0-4095) para coordenadas de tela (0-479 x 0-319)
        // FT6336U retorna 12-bit raw (0-4095) em ambos os eixos.
        // Fórmula de escala: mapped = (raw * screen_dim) / 4095
        
        // Clamp raw values ao intervalo 12-bit válido
        uint16_t raw_x = (p.x > 4095) ? 4095 : p.x;
        uint16_t raw_y = (p.y > 4095) ? 4095 : p.y;

        // Se o controlador já reporta coordenadas em pixels, não reescalar.
        // Caso contrário, escala de 12-bit para resolução da tela.
        uint32_t scaled_x = 0;
        uint32_t scaled_y = 0;
        bool raw_is_pixels = (raw_x < w) && (raw_y < h);
        if (raw_is_pixels) {
            scaled_x = raw_x;
            scaled_y = raw_y;
        } else {
            scaled_x = (raw_x * (uint32_t)w) / 4095;
            scaled_y = (raw_y * (uint32_t)h) / 4095;
        }
        
        // Clamp to screen bounds
        x = (scaled_x >= w) ? (uint16_t)(w - 1u) : (uint16_t)scaled_x;
        y = (scaled_y >= h) ? (uint16_t)(h - 1u) : (uint16_t)scaled_y;

        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PR;

        // Guarda último ponto para soltar no mesmo local
        last_x = x;
        last_y = y;

        if (!last_pressed) {
            Logger::info("[TOUCH] Press raw=(%u,%u) scaled=(%u,%u) mode=%s", 
                         (unsigned)raw_x, (unsigned)raw_y, (unsigned)x, (unsigned)y,
                         raw_is_pixels ? "pixels" : "12bit");
        }
        last_pressed = true;
    } else {
        // Reutiliza última posição conhecida para evitar salto visual no release
        data->point.x = last_x;
        data->point.y = last_y;
        data->state = LV_INDEV_STATE_REL;
        if (last_pressed) {
            Logger::info("[TOUCH] Release");
        }
        last_pressed = false;
    }
}