#include "tasks/task_lvgl.hpp"
#include "tasks/task_wifi_time_init.hpp"
#include "core/state_machine.hpp"
#include "ui/screens/welcome_screen.hpp"
#include "ui/screens/init_status_screen.hpp"
#include "ui/screens/appointment_screen.hpp"
#include "ui/screens/error_screen.hpp"
#include "ui/screens/error_critical_screen.hpp"
#include "services/time_service.hpp"

#include <lvgl.h>
#include <stdio.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

#define DEMO_MODE 1

#if DEMO_MODE
  #define DEMO_WAIT_IDLE_MS 3000   // espera 3s no IDLE antes de simular sucesso
  #define DEMO_SUCCESS_MS   5000   // mostra sucesso por 5s antes de voltar
  static TickType_t demo_t0 = 0;
  static bool demo_running = false;
  static bool demo_showing_success = false;
#endif

void task_lvgl(void* params) {
    (void)params;

    printf("[LVGL Task] Started\n");

    init_status_screen_show("Inicializando Sistema", "Preparando...");

    // Sinaliza que o LVGL já está de pé (atenção: isso NÃO deveria autorizar outras tasks a chamarem lv_*)
    xEventGroupSetBits(wifi_time_ready_group_event, LVGL_READY_BIT);

    StateMachine::State last_state = StateMachine::State::INITIALIZING;
    uint32_t time_update_counter = 0;

    while (1) {
        // Processa LVGL (timers, render, animações)
        lv_timer_handler();

        StateMachine::State current_state = StateMachine::getInstance().getState();

        // Troca de telas quando o estado muda
        if (current_state != last_state) {
            printf("[LVGL] %s -> %s\n",
                   StateMachine::getInstance().getStateName(last_state),
                   StateMachine::getInstance().getStateName(current_state));

            switch (current_state) {
                case StateMachine::State::INITIALIZING:
                    init_status_screen_show("Inicializando Sistema", "Preparando...");
                    break;

                case StateMachine::State::IDLE:
                    welcome_screen_show(nullptr);
                    break;

                case StateMachine::State::APPOINTMENT:
                    appointment_screen_show();
                    break;

                case StateMachine::State::ERROR:
                    error_screen_show();
                    break;

                case StateMachine::State::ERROR_CRITICAL:
                    error_critical_screen_show();
                    break;

                case StateMachine::State::RESTARTING:
                    error_critical_screen_clear();
                    break;

                default:
                    break;
            }

            last_state = current_state;
        }

        // Atualização periódica do relógio (a cada ~1s: 50 * 20ms)
        time_update_counter++;
        if (time_update_counter >= 50) {
            time_update_counter = 0;

            switch (current_state) {
                case StateMachine::State::IDLE:
                case StateMachine::State::VALIDATING:
                case StateMachine::State::ERROR:
                    welcome_screen_update_time();
                    break;

                default:
                    break;
            }
        }

#if DEMO_MODE
        // ---- DEMO MODE: simula check-in automaticamente ----
        // Arma demo ao entrar no IDLE
        if (current_state == StateMachine::State::IDLE && !demo_running) {
            demo_running = true;
            demo_showing_success = false;
            demo_t0 = xTaskGetTickCount();
            printf("[DEMO] Armed in IDLE\n");
        }

        // Se sair do IDLE pra estados "não relacionados", desarma (evita comportamento estranho)
        if (current_state != StateMachine::State::IDLE &&
            current_state != StateMachine::State::APPOINTMENT) {
            if (demo_running) {
                printf("[DEMO] Disarmed (state changed)\n");
            }
            demo_running = false;
            demo_showing_success = false;
        }

        if (demo_running) {
            TickType_t now = xTaskGetTickCount();

            // Depois de X ms no IDLE, dispara sucesso
            if (!demo_showing_success && current_state == StateMachine::State::IDLE) {
                if ((now - demo_t0) >= pdMS_TO_TICKS(DEMO_WAIT_IDLE_MS)) {
                    demo_showing_success = true;
                    demo_t0 = now;

                    printf("[DEMO] Triggering success screen\n");

                    // Vai para a tela de sucesso
                    StateMachine::getInstance().setState(StateMachine::State::APPOINTMENT);

                    // Atualiza os dados exibidos (depois de setState, a tela será mostrada)
                    appointment_screen_update(
                        "Adriana Rocha",
                        "CPF: 123.456.789-01",
                        "Horario: 16:45",
                        "Dra. Rosa Maria - Clinica Geral"
                    );
                }
            }

            // Depois de Y ms na tela de sucesso, volta pro IDLE
            if (demo_showing_success && current_state == StateMachine::State::APPOINTMENT) {
                if ((now - demo_t0) >= pdMS_TO_TICKS(DEMO_SUCCESS_MS)) {
                    printf("[DEMO] Returning to IDLE\n");

                    demo_running = false;
                    demo_showing_success = false;
                    StateMachine::getInstance().setState(StateMachine::State::IDLE);
                }
            }
        }
        // ---- /DEMO MODE ----
#endif

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}