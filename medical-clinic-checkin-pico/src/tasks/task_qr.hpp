#pragma once
#include "hal/interfaces/hal_uart_interface.hpp"
#include "core/checkin_state_machine.hpp"

class TaskQR {
public:
    // O erro estava aqui. Precisa do ';' no final desta linha:
    TaskQR(HAL_UART_Interface* uart, CheckinStateMachine* fsm);
    
    void run();

private:
    HAL_UART_Interface* uart_;
    CheckinStateMachine* fsm_;
};