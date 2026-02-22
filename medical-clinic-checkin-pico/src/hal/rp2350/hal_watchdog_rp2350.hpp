#pragma once
#ifndef HAL_WATCHDOG_RP2350_HPP
#define HAL_WATCHDOG_RP2350_HPP

#include <cstdint>
#include "hal/interfaces/hal_watchdog_interface.hpp"

class HAL_Watchdog_RP2350 : public HAL_Watchdog_Interface {
public:
    void init(uint32_t timeout_ms) override;
    void feed() override;
    void reboot() override;
};

#endif // HAL_WATCHDOG_RP2350_HPP
