#pragma once
#include <cstddef>
#include <cstdint>

class RingBuffer;

class HAL_UART_Interface {
public:
    virtual ~HAL_UART_Interface() = default;

    // Métodos Puros (Obrigatórios para quem implementar)
    virtual bool init(uint32_t baudrate) = 0;
    virtual void deinit() = 0;
    virtual size_t write(const uint8_t* data, size_t len) = 0;
    virtual size_t available() = 0;
    virtual bool read_byte(uint8_t* out) = 0;
    virtual uint8_t readByte() = 0;
    virtual void enable_rx_irq(RingBuffer* rx_buffer) = 0;
    virtual void disable_rx_irq() = 0;

    // --- MÉTODOS HELPERS (ADICIONADOS) ---
    // Isso resolve o erro: "class HAL_UART_Interface has no member named 'read'"
    
    virtual size_t read(uint8_t* buf, size_t len) {
        if (!buf || len == 0) return 0;
        size_t count = 0;
        while (count < len && available() > 0) {
            buf[count++] = readByte();
        }
        return count;
    }
};