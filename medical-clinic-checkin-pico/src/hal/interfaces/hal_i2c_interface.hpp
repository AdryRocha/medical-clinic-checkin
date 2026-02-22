#pragma once
#include <cstdint>
#include <cstddef>

class HAL_I2C_Interface {
public:
    virtual ~HAL_I2C_Interface() = default;

    virtual bool init(uint32_t speed) = 0;
    virtual bool write(uint8_t addr, const uint8_t *data, size_t len) = 0;
    virtual bool read(uint8_t addr, uint8_t *data, size_t len) = 0;

    virtual bool write_reg(uint8_t addr, uint8_t reg, uint8_t value) = 0;
    virtual bool read_reg(uint8_t addr, uint8_t reg, uint8_t *value) = 0;

    virtual bool write_read(uint8_t addr,
                            const uint8_t *wdata, size_t wlen,
                            uint8_t *rdata, size_t rlen) = 0;
};
