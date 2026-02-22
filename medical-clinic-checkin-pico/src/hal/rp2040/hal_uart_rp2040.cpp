#include "hal_uart_rp2040.hpp"
#include "pico/stdlib.h"

HAL_UART_RP2040::HAL_UART_RP2040(uart_inst_t* uart_id, uint8_t tx_pin, uint8_t rx_pin)
    : uart_inst_(uart_id), tx_pin_(tx_pin), rx_pin_(rx_pin) {
}

bool HAL_UART_RP2040::init(uint32_t baudrate, uint8_t data_bits, uint8_t stop_bits, uint8_t parity) {
    // Inicializa a UART com o baudrate base
    uart_init(uart_inst_, baudrate);

    // Configura os pinos TX e RX
    gpio_set_function(tx_pin_, GPIO_FUNC_UART);
    gpio_set_function(rx_pin_, GPIO_FUNC_UART);

    // Traduz configuração de paridade para o SDK do Pico
    uart_parity_t sdk_parity = UART_PARITY_NONE;
    if (parity == 1) sdk_parity = UART_PARITY_ODD;
    else if (parity == 2) sdk_parity = UART_PARITY_EVEN;

    // Aplica formato (Data bits, Stop bits, Parity)
    uart_set_format(uart_inst_, data_bits, stop_bits, sdk_parity);

    // Habilita FIFOs (importante para performance)
    uart_set_fifo_enabled(uart_inst_, true);

    return true;
}

size_t HAL_UART_RP2040::write(const uint8_t* data, size_t length) {
    if (!uart_is_enabled(uart_inst_)) return 0;
    
    // Bloqueante: escreve tudo
    uart_write_blocking(uart_inst_, data, length);
    return length;
}

size_t HAL_UART_RP2040::read(uint8_t* data, size_t length) {
    if (!uart_is_readable(uart_inst_)) return 0;

    size_t read_count = 0;
    while (read_count < length && uart_is_readable(uart_inst_)) {
        data[read_count++] = uart_getc(uart_inst_);
    }
    return read_count;
}

void HAL_UART_RP2040::writeByte(uint8_t byte) {
    uart_putc_raw(uart_inst_, byte);
}

uint8_t HAL_UART_RP2040::readByte() {
    return uart_getc(uart_inst_);
}

size_t HAL_UART_RP2040::available() {
    return uart_is_readable(uart_inst_) ? 1 : 0; 
    // Nota: O SDK do Pico não expõe facilmente quantos bytes estão no FIFO exatos,
    // mas uart_is_readable retorna true se houver pelo menos 1.
}

void HAL_UART_RP2040::flush() {
    // Aguarda TX terminar
    uart_tx_wait_blocking(uart_inst_);
}

void HAL_UART_RP2040::clearRxBuffer() {
    // Esvazia o FIFO de leitura
    while (uart_is_readable(uart_inst_)) {
        volatile uint8_t dummy = uart_getc(uart_inst_);
        (void)dummy;
    }
}