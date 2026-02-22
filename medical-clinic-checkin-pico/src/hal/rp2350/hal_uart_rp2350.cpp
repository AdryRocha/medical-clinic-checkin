#include "hal/rp2350/hal_uart_rp2350.hpp"
#include "pico/stdlib.h"

// Instâncias estáticas para IRQ
static HAL_UART_RP2350* uart0_instance = nullptr;
static HAL_UART_RP2350* uart1_instance = nullptr;

static void on_uart0_rx() { if (uart0_instance) uart0_instance->irq_handler(); }
static void on_uart1_rx() { if (uart1_instance) uart1_instance->irq_handler(); }

HAL_UART_RP2350::HAL_UART_RP2350(uart_inst_t* uart, uint tx_pin, uint rx_pin, int irq, RingBuffer* rx_buffer)
    : uart_(uart), tx_pin_(tx_pin), rx_pin_(rx_pin), irq_(irq), rx_buffer_(rx_buffer) 
{
    if (uart == uart0) uart0_instance = this;
    else if (uart == uart1) uart1_instance = this;
}

bool HAL_UART_RP2350::init(uint32_t baudrate) {
    uart_init(uart_, baudrate);
    gpio_set_function(tx_pin_, GPIO_FUNC_UART);
    gpio_set_function(rx_pin_, GPIO_FUNC_UART);
    
    // Configuração padrão 8N1
    uart_set_hw_flow(uart_, false, false);
    uart_set_format(uart_, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart_, false);

    if (rx_buffer_) {
        int irq_num = (uart_ == uart0) ? UART0_IRQ : UART1_IRQ;
        irq_set_exclusive_handler(irq_num, (uart_ == uart0) ? on_uart0_rx : on_uart1_rx);
        irq_set_enabled(irq_num, true);
        uart_set_irq_enables(uart_, true, false);
    }
    return true;
}

size_t HAL_UART_RP2350::write(const uint8_t* data, size_t len) {
    if (!uart_is_writable(uart_)) return 0;
    for(size_t i=0; i<len; i++) uart_putc(uart_, data[i]);
    return len;
}

size_t HAL_UART_RP2350::available() {
    return rx_buffer_ ? rx_buffer_->available() : (uart_is_readable(uart_) ? 1 : 0);
}

bool HAL_UART_RP2350::read_byte(uint8_t* out) {
    if (rx_buffer_) return rx_buffer_->pop(*out);
    if (uart_is_readable(uart_)) {
        *out = uart_getc(uart_);
        return true;
    }
    return false;
}

uint8_t HAL_UART_RP2350::readByte() {
    uint8_t b = 0;
    read_byte(&b);
    return b;
}

void HAL_UART_RP2350::enable_rx_irq(RingBuffer* rx_buffer) {
    rx_buffer_ = rx_buffer; // Reativa buffer se necessário
}

void HAL_UART_RP2350::disable_rx_irq() {
    uart_set_irq_enables(uart_, false, false);
}

void HAL_UART_RP2350::flush() {}
void HAL_UART_RP2350::clearRxBuffer() { if(rx_buffer_) rx_buffer_->clear(); }
void HAL_UART_RP2350::writeByte(uint8_t byte) { uart_putc(uart_, byte); }

void HAL_UART_RP2350::irq_handler() {
    while (uart_is_readable(uart_)) {
        uint8_t ch = uart_getc(uart_);
        if (rx_buffer_) rx_buffer_->push(ch);
    }
}

void HAL_UART_RP2350::deinit() {
    if (!uart_) return;

    uart_deinit(uart_);

    int irq_num = (uart_ == uart0) ? UART0_IRQ : UART1_IRQ;
    irq_set_enabled(irq_num, false);

    // Remova handler apropriado se você tiver handlers estáticos/funcionais
    // Substitua `on_uart0_rx` / `on_uart1_rx` pelos nomes reais usados no seu código
    irq_remove_handler(irq_num, (uart_ == uart0) ? on_uart0_rx : on_uart1_rx);

    if (uart_ == uart0) uart0_instance = nullptr;
    else if (uart_ == uart1) uart1_instance = nullptr;

    uart_ = nullptr;
}