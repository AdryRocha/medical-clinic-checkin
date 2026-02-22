#include "tasks/task_fsm.hpp"
#include "fsm/checkin_fsm.hpp"
#include "services/logger_service.hpp"

void task_fsm(void* arg)
{
    (void)arg;

    LOGGER_INFO("[FSM] Task iniciada");

    while (true)
    {
        CheckinEvent evt;
        char payload[128];

        if (fsm_wait_event(&evt, payload, sizeof(payload)))
        {
            LOGGER_INFO("[FSM] Evento recebido: %d", (int)evt);
            checkin_fsm_handle_event(evt, payload);
        }

        vTaskDelay(pdMS_TO_TICKS(5));

        // Log periódico de loop ativo e stack
        static TickType_t last_log = 0;
        if ((xTaskGetTickCount() - last_log) > pdMS_TO_TICKS(2000)) {
            TaskHandle_t this_task = xTaskGetCurrentTaskHandle();
            UBaseType_t stack_min = uxTaskGetStackHighWaterMark(this_task);
            LOGGER_INFO("[FSM] loop ativo, heap livre: %u, stack min: %u words", xPortGetFreeHeapSize(), (unsigned)stack_min);
            last_log = xTaskGetTickCount();
        }
    }
}
