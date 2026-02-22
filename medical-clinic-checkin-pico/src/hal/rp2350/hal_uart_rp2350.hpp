#pragma once
#include "hal/interfaces/hal_uart_interface.hpp"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "utils/ring_buffer.hpp"

class HAL_UART_RP2350 : public HAL_UART_Interface {
public:
    // Construtor
    HAL_UART_RP2350(uart_inst_t* uart, uint tx_pin, uint rx_pin, int irq, RingBuffer* rx_buffer = nullptr);

    // Overrides obrigatórios
    bool init(uint32_t baudrate) override;
    void deinit() override;

    size_t write(const uint8_t* data, size_t len) override;
    size_t available() override;
    bool read_byte(uint8_t* out) override;
    uint8_t readByte() override;
    
    void enable_rx_irq(RingBuffer* rx_buffer) override;
    void disable_rx_irq() override;

    // --- MÉTODOS EXTRAS (Adicionados para resolver o erro) ---
    void flush();
    void clearRxBuffer();
    void writeByte(uint8_t byte);

    // Handler de interrupção
    void irq_handler();

private:
    uart_inst_t* uart_;
    uint tx_pin_;
    uint rx_pin_;
    int irq_;
    RingBuffer* rx_buffer_;
};