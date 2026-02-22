#ifndef HAL_UART_RP2040_HPP
#define HAL_UART_RP2040_HPP

#include "hal/interfaces/hal_uart_interface.hpp"
#include "hardware/uart.h"
#include "hardware/gpio.h"

/**
 * @brief RP2040-specific UART HAL implementation
 */
class HAL_UART_RP2040 : public HAL_UART_Interface {
private:
    uart_inst_t* uart_instance_;
    uint8_t pin_tx_;
    uint8_t pin_rx_;
    uint32_t baudrate_;

public:
    /**
     * @brief Construct an RP2040 UART HAL instance
     * @param uart_instance UART peripheral (uart0 or uart1)
     * @param pin_tx TX pin number
     * @param pin_rx RX pin number
     */
    HAL_UART_RP2040(uart_inst_t* uart_instance, uint8_t pin_tx, uint8_t pin_rx);

    bool init(uint32_t baudrate, uint8_t data_bits = 8, 
             uint8_t stop_bits = 1, uint8_t parity = 0) override;
    
    size_t write(const uint8_t* data, size_t length) override;
    size_t read(uint8_t* data, size_t length) override;
    size_t available() override;
    void writeByte(uint8_t byte) override;
    uint8_t readByte() override;
    void flush() override;
    void clearRxBuffer() override;
    size_t readAvailable(uint8_t* data, size_t max_length) override;
};

#endif // HAL_UART_RP2040_HPP