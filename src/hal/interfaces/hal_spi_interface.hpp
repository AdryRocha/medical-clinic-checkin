#ifndef HAL_SPI_INTERFACE_HPP
#define HAL_SPI_INTERFACE_HPP

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Hardware Abstraction Layer for SPI communication
 */
class HAL_SPI_Interface {
public:
    virtual ~HAL_SPI_Interface() = default;

    /**
     * @brief Initialize the SPI peripheral
     * @param baudrate SPI clock speed in Hz
     * @return true if initialization successful
     */
    virtual bool init(uint32_t baudrate) = 0;

    /**
     * @brief Write data to SPI
     * @param data Pointer to data buffer
     * @param len Number of bytes to write
     * @return Number of bytes written
     */
    virtual size_t write(const uint8_t* data, size_t len) = 0;

    /**
     * @brief Read data from SPI
     * @param data Pointer to receive buffer
     * @param len Number of bytes to read
     * @return Number of bytes read
     */
    virtual size_t read(uint8_t* data, size_t len) = 0;

    /**
     * @brief Set chip select pin state
     * @param state true for select (low), false for deselect (high)
     */
    virtual void setCS(bool state) = 0;

    /**
     * @brief Set data/command pin state
     * @param state true for data, false for command
     */
    virtual void setDC(bool state) = 0;

    /**
     * @brief Perform hardware reset
     */
    virtual void reset() = 0;
};

#endif // HAL_SPI_INTERFACE_HPP