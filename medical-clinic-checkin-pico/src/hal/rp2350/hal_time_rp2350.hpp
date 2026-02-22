#pragma once
#include <cstdint>

#include "hal/interfaces/hal_time_interface.hpp"

class HAL_Time_RP2350 : public HAL_Time_Interface {
public:
    uint64_t millis() override;
    uint64_t micros() override;
    void delay_ms(uint32_t ms) override;
};
