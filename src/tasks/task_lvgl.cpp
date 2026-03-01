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

void task_lvgl(void* params) {
    (void)params;
    
    printf("[LVGL Task] Started\n");
    
    init_status_screen_show("Inicializando Sistema", "Preparando...");
    
    xEventGroupSetBits(wifi_time_ready_group_event, LVGL_READY_BIT);
    
    StateMachine::State last_state = StateMachine::State::INITIALIZING;
    uint32_t time_update_counter = 0;
    
    while (1) {
        lv_timer_handler();
        
        StateMachine::State current_state = StateMachine::getInstance().getState();
        
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
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}