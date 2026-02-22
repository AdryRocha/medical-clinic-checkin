#ifndef HAL_I2C_INTERFACE_HPP
#define HAL_I2C_INTERFACE_HPP

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Hardware Abstraction Layer for I2C communication
 */
class HAL_I2C_Interface {
public:
    virtual ~HAL_I2C_Interface() = default;

    /**
     * @brief Initialize the I2C peripheral
     * @param baudrate I2C bus speed in Hz
     * @return true if initialization was successful
     */
    virtual bool init(uint32_t baudrate) = 0;

    /**
     * @brief Write data to an I2C device
     * @param address 7-bit I2C device address
     * @param data Pointer to data buffer
     * @param length Number of bytes to write
     * @param sendStop If true, send STOP condition after write
     * @return true if write succeeded
     */
    virtual bool write(uint8_t address, const uint8_t* data, size_t length, bool sendStop = true) = 0;

    /**
     * @brief Read data from an I2C device
     * @param address 7-bit I2C device address
     * @param data Pointer to buffer for received data
     * @param length Number of bytes to read
     * @param sendStop If true, send STOP condition after read
     * @return true if read succeeded
     */
    virtual bool read(uint8_t address, uint8_t* data, size_t length, bool sendStop = true) = 0;

    /**
     * @brief Perform a combined write followed by read without releasing the bus
     * @param address 7-bit I2C device address
     * @param writeData Pointer to data to write
     * @param writeLength Number of bytes to write
     * @param readData Pointer to buffer for read data
     * @param readLength Number of bytes to read
     * @return true if both operations succeed
     */
    virtual bool writeRead(uint8_t address,
                           const uint8_t* writeData,
                           size_t writeLength,
                           uint8_t* readData,
                           size_t readLength) = 0;
};

#endif // HAL_I2C_INTERFACE_HPP
