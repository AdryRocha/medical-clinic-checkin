#include "hal_mutex_freertos.hpp"

HAL_Mutex_FreeRTOS::HAL_Mutex_FreeRTOS() {
    mux_ = xSemaphoreCreateMutex();
}

bool HAL_Mutex_FreeRTOS::lock() {
    return xSemaphoreTake(mux_, portMAX_DELAY) == pdTRUE;
}

void HAL_Mutex_FreeRTOS::unlock() {
    xSemaphoreGive(mux_);
}
