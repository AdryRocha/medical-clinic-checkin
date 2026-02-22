#pragma once

#include <cstddef>
#include <cstdint>

#include "hal/interfaces/hal_i2c_interface.hpp"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

class HAL_I2C_RP2350 : public HAL_I2C_Interface {
public:
    HAL_I2C_RP2350(i2c_inst_t* inst, uint sda, uint scl);

    bool init(uint32_t speed) override;

    bool write(uint8_t addr, const uint8_t* data, size_t len) override;
    bool read(uint8_t addr, uint8_t* data, size_t len) override;

    bool write_reg(uint8_t addr, uint8_t reg, uint8_t value) override;
    bool read_reg(uint8_t addr, uint8_t reg, uint8_t* value) override;

    bool write_read(uint8_t addr,
                    const uint8_t* wdata, size_t wlen,
                    uint8_t* rdata, size_t rlen) override;

private:
    i2c_inst_t* i2c_;
    uint sda_;
    uint scl_;
};
