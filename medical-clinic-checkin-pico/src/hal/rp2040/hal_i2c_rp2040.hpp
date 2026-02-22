#ifndef HAL_I2C_RP2040_HPP
#define HAL_I2C_RP2040_HPP

#include "hal/interfaces/hal_i2c_interface.hpp"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

/**
 * @brief RP2040-specific I2C HAL implementation
 */
class HAL_I2C_RP2040 : public HAL_I2C_Interface {
private:
    i2c_inst_t* i2c_instance_;
    uint8_t pin_sda_;
    uint8_t pin_scl_;

public:
    /**
     * @brief Construct an RP2040 I2C HAL instance
     * @param i2c_instance I2C peripheral (i2c0 or i2c1)
     * @param pin_sda SDA pin number
     * @param pin_scl SCL pin number
     */
    HAL_I2C_RP2040(i2c_inst_t* i2c_instance, uint8_t pin_sda, uint8_t pin_scl);

    bool init(uint32_t baudrate) override;
    bool write(uint8_t address, const uint8_t* data, size_t length, bool sendStop = true) override;
    bool read(uint8_t address, uint8_t* data, size_t length, bool sendStop = true) override;
    bool writeRead(uint8_t address,
                   const uint8_t* writeData,
                   size_t writeLength,
                   uint8_t* readData,
                   size_t readLength) override;
};

#endif // HAL_I2C_RP2040_HPP
