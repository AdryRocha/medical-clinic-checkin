#pragma once
#include <cstdint>

class HAL_Time_Interface {
public:
    virtual ~HAL_Time_Interface() = default;

    virtual uint64_t millis() = 0;
    virtual uint64_t micros() = 0;
    virtual void delay_ms(uint32_t ms) = 0;
};
