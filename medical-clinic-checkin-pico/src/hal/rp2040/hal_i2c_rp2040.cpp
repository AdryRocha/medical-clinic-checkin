#include "hal_i2c_rp2040.hpp"
#include "pico/stdlib.h"

HAL_I2C_RP2040::HAL_I2C_RP2040(i2c_inst_t* i2c_instance, uint8_t pin_sda, uint8_t pin_scl)
    : i2c_instance_(i2c_instance),
      pin_sda_(pin_sda),
      pin_scl_(pin_scl) {}

bool HAL_I2C_RP2040::init(uint32_t baudrate) {
    if (i2c_instance_ == nullptr) {
        return false;
    }

    i2c_init(i2c_instance_, baudrate);

    gpio_set_function(pin_sda_, GPIO_FUNC_I2C);
    gpio_pull_up(pin_sda_);

    gpio_set_function(pin_scl_, GPIO_FUNC_I2C);
    gpio_pull_up(pin_scl_);

    return true;
}

bool HAL_I2C_RP2040::write(uint8_t address, const uint8_t* data, size_t length, bool sendStop) {
    if (data == nullptr || length == 0) {
        return false;
    }

    int result = i2c_write_blocking(i2c_instance_, address, data, length, !sendStop);
    return result == static_cast<int>(length);
}

bool HAL_I2C_RP2040::read(uint8_t address, uint8_t* data, size_t length, bool sendStop) {
    if (data == nullptr || length == 0) {
        return false;
    }

    int result = i2c_read_blocking(i2c_instance_, address, data, length, !sendStop);
    return result == static_cast<int>(length);
}

bool HAL_I2C_RP2040::writeRead(uint8_t address,
                               const uint8_t* writeData,
                               size_t writeLength,
                               uint8_t* readData,
                               size_t readLength) {
    if (writeData == nullptr || writeLength == 0 || readData == nullptr || readLength == 0) {
        return false;
    }

    int writeResult = i2c_write_blocking(i2c_instance_, address, writeData, writeLength, true);
    if (writeResult != static_cast<int>(writeLength)) {
        return false;
    }

    int readResult = i2c_read_blocking(i2c_instance_, address, readData, readLength, false);
    return readResult == static_cast<int>(readLength);
}
