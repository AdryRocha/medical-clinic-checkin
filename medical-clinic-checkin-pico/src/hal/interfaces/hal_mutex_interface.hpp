#pragma once

class HAL_Mutex_Interface {
public:
    virtual ~HAL_Mutex_Interface() = default;
    virtual bool lock() = 0;
    virtual void unlock() = 0;
};