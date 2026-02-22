#pragma once
#include "hal/interfaces/hal_power_interface.hpp"

class HAL_Power_RP2350 : public HAL_Power_Interface {
public:
    void reboot() override;
    void sleep() override;
};