#include "hal_uart_rp2040.hpp"
#include "pico/stdlib.h"

HAL_UART_RP2040::HAL_UART_RP2040(uart_inst_t* uart_instance, uint8_t pin_tx, uint8_t pin_rx)
    : uart_instance_(uart_instance),
      pin_tx_(pin_tx),
      pin_rx_(pin_rx),
      baudrate_(0) {
}

bool HAL_UART_RP2040::init(uint32_t baudrate, uint8_t data_bits, 
                           uint8_t stop_bits, uint8_t parity) {
    if (uart_instance_ == nullptr) {
        return false;
    }

    uint32_t actual_baudrate = uart_init(uart_instance_, baudrate);
    baudrate_ = actual_baudrate;

    uart_parity_t parity_mode = UART_PARITY_NONE;
    if (parity == 1) {
        parity_mode = UART_PARITY_ODD;
    } else if (parity == 2) {
        parity_mode = UART_PARITY_EVEN;
    }

    uart_set_format(uart_instance_, data_bits, stop_bits, parity_mode);

    gpio_set_function(pin_tx_, GPIO_FUNC_UART);
    gpio_set_function(pin_rx_, GPIO_FUNC_UART);

    uart_set_fifo_enabled(uart_instance_, true);

    return true;
}

size_t HAL_UART_RP2040::write(const uint8_t* data, size_t length) {
    if (data == nullptr || length == 0) {
        return 0;
    }

    uart_write_blocking(uart_instance_, data, length);
    return length;
}

size_t HAL_UART_RP2040::read(uint8_t* data, size_t length) {
    if (data == nullptr || length == 0) {
        return 0;
    }

    size_t bytes_read = 0;
    while (bytes_read < length && uart_is_readable(uart_instance_)) {
        data[bytes_read++] = uart_getc(uart_instance_);
    }

    return bytes_read;
}

size_t HAL_UART_RP2040::available() {
    return uart_is_readable(uart_instance_) ? 1 : 0;
}

void HAL_UART_RP2040::writeByte(uint8_t byte) {
    uart_putc_raw(uart_instance_, byte);
}

uint8_t HAL_UART_RP2040::readByte() {
    while (!uart_is_readable(uart_instance_)) {
        tight_loop_contents();
    }
    return uart_getc(uart_instance_);
}

void HAL_UART_RP2040::flush() {
    uart_tx_wait_blocking(uart_instance_);
}

void HAL_UART_RP2040::clearRxBuffer() {
    while (uart_is_readable(uart_instance_)) {
        uart_getc(uart_instance_);
    }
}

size_t HAL_UART_RP2040::readAvailable(uint8_t* data, size_t max_length) {
    if (data == nullptr || max_length == 0) {
        return 0;
    }

    size_t bytes_read = 0;
    while (bytes_read < max_length && uart_is_readable(uart_instance_)) {
        data[bytes_read++] = uart_getc(uart_instance_);
    }

    return bytes_read;
}