#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "drivers/display/interface/display_interface.hpp"
#include "drivers/touch/interface/touch_interface.hpp"

class TaskUI {
public:
    TaskUI(DisplayInterface* d, TouchInterface* t);

    static void taskEntry(void* arg);
    void run();

private:
    DisplayInterface* display_;
    TouchInterface* touch_;
};
