#ifndef HAL_UART_RP2040_HPP
#define HAL_UART_RP2040_HPP

#include "hal/interfaces/hal_uart_interface.hpp"
#include "hardware/uart.h"
#include "hardware/gpio.h"

/**
 * @brief Implementação da HAL UART para Raspberry Pi Pico (RP2040/RP2350)
 */
class HAL_UART_RP2040 : public HAL_UART_Interface {
private:
    uart_inst_t* uart_inst_;
    uint8_t tx_pin_;
    uint8_t rx_pin_;

public:
    /**
     * @param uart_id uart0 ou uart1
     * @param tx_pin GPIO para TX
     * @param rx_pin GPIO para RX
     */
    HAL_UART_RP2040(uart_inst_t* uart_id, uint8_t tx_pin, uint8_t rx_pin);

    bool init(uint32_t baudrate, uint8_t data_bits = 8,
              uint8_t stop_bits = 1, uint8_t parity = 0) override;

    size_t write(const uint8_t* data, size_t length) override;
    size_t read(uint8_t* data, size_t length) override;
    
    // Funções byte a byte (necessárias para o parser dos drivers)
    void writeByte(uint8_t byte) override;
    uint8_t readByte() override;
    
    size_t available() override;
    void flush() override;
    void clearRxBuffer() override;
};

#endif // HAL_UART_RP2040_HPP