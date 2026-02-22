#include "services/logger_service.hpp"
#include "FreeRTOS.h"
#include "task.h"

void task_sdcard_entry(void* arg) {
    Logger::info("[SDCARD] Task iniciada.");
    while (true) {
        Logger::info("[SDCARD] loop ativo, heap livre: %u", xPortGetFreeHeapSize());
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
