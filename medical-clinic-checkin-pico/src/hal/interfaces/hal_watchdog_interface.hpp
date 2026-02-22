#pragma once
#ifndef HAL_WATCHDOG_INTERFACE_HPP
#define HAL_WATCHDOG_INTERFACE_HPP

#include <cstdint>

class HAL_Watchdog_Interface {
public:
    virtual ~HAL_Watchdog_Interface() = default;

    virtual void init(uint32_t timeout_ms) = 0;
    virtual void feed() = 0;
    virtual void reboot() = 0;
};

#endif // HAL_WATCHDOG_INTERFACE_HPP
