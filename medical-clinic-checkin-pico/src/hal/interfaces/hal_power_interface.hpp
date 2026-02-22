#pragma once

class HAL_Power_Interface {
public:
    virtual ~HAL_Power_Interface() = default;

    virtual void reboot() = 0;
    virtual void sleep() = 0; // Adicionado para resolver o erro de override
};