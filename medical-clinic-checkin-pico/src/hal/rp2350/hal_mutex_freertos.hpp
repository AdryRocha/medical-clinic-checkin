#pragma once
#include "hal/interfaces/hal_mutex_interface.hpp"
#include "FreeRTOS.h"
#include "semphr.h"

class HAL_Mutex_FreeRTOS : public HAL_Mutex_Interface {
public:
    HAL_Mutex_FreeRTOS();
    bool lock() override;
    void unlock() override;

private:
    SemaphoreHandle_t mux_;
};
