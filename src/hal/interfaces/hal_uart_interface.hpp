#ifndef HAL_UART_INTERFACE_HPP
#define HAL_UART_INTERFACE_HPP

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Hardware Abstraction Layer for UART communication
 */
class HAL_UART_Interface {
public:
    virtual ~HAL_UART_Interface() = default;

    /**
     * @brief Initialize the UART peripheral with communication parameters
     * @note This should only be called by the application layer, not drivers.
     *       Drivers should receive pre-initialized HAL instances.
     * @param baudrate UART baud rate
     * @param data_bits Number of data bits (5-8)
     * @param stop_bits Number of stop bits (1 or 2)
     * @param parity Parity mode (0=none, 1=odd, 2=even)
     * @return true if initialization successful
     */
    virtual bool init(uint32_t baudrate, uint8_t data_bits = 8,
                     uint8_t stop_bits = 1, uint8_t parity = 0) = 0;

    /**
     * @brief Write data to UART
     * @param data Pointer to data buffer
     * @param length Number of bytes to write
     * @return Number of bytes written
     */
    virtual size_t write(const uint8_t* data, size_t length) = 0;

    /**
     * @brief Read data from UART
     * @param data Pointer to receive buffer
     * @param length Maximum number of bytes to read
     * @return Number of bytes read
     */
    virtual size_t read(uint8_t* data, size_t length) = 0;

    /**
     * @brief Check if data is available to read
     * @return Number of bytes available
     */
    virtual size_t available() = 0;

    /**
     * @brief Write a single byte to UART (blocking)
     * @param byte Byte to write
     */
    virtual void writeByte(uint8_t byte) = 0;

    /**
     * @brief Read a single byte from UART (blocking)
     * @return Byte read
     */
    virtual uint8_t readByte() = 0;

    /**
     * @brief Flush TX buffer
     */
    virtual void flush() = 0;

    /**
     * @brief Clear RX buffer
     */
    virtual void clearRxBuffer() = 0;

    /**
     * @brief Read all available bytes (non-blocking)
     * @param data Pointer to receive buffer
     * @param max_length Maximum number of bytes to read
     * @return Number of bytes read
     */
    virtual size_t readAvailable(uint8_t* data, size_t max_length) = 0;

};

#endif // HAL_UART_INTERFACE_HPP
