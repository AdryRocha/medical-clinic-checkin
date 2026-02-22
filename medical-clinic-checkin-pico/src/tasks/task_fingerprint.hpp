#pragma once
#include "hal/interfaces/hal_uart_interface.hpp"
#include "utils/ring_buffer.hpp"

class TaskFingerprint {
public:
    // CORREÇÃO: Adicionado o RingBuffer* no contrato da classe
    TaskFingerprint(HAL_UART_Interface* uart, RingBuffer* buffer);
    
    void run();

private:
    bool decodePacket();

    HAL_UART_Interface* uart_;
    RingBuffer* buffer_; // Ponteiro para o buffer estático
};