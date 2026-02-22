#include "hal_i2c_rp2350.hpp"

HAL_I2C_RP2350::HAL_I2C_RP2350(i2c_inst_t* inst, uint sda, uint scl)
    : i2c_(inst), sda_(sda), scl_(scl) {}

bool HAL_I2C_RP2350::init(uint32_t speed) {
    i2c_init(i2c_, speed);

    gpio_set_function(sda_, GPIO_FUNC_I2C);
    gpio_set_function(scl_, GPIO_FUNC_I2C);
    gpio_pull_up(sda_);
    gpio_pull_up(scl_);

    return true;
}

bool HAL_I2C_RP2350::write(uint8_t addr, const uint8_t* data, size_t len) {
    if (!data || len == 0) return true;
    int ret = i2c_write_blocking(i2c_, addr, data, (int)len, false);
    return ret == (int)len;
}

bool HAL_I2C_RP2350::read(uint8_t addr, uint8_t* data, size_t len) {
    if (!data || len == 0) return true;
    int ret = i2c_read_blocking(i2c_, addr, data, (int)len, false);
    return ret == (int)len;
}

bool HAL_I2C_RP2350::write_reg(uint8_t addr, uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    return write(addr, buf, 2);
}

bool HAL_I2C_RP2350::read_reg(uint8_t addr, uint8_t reg, uint8_t* value) {
    if (!value) return false;
    int ret1 = i2c_write_blocking(i2c_, addr, &reg, 1, true);
    if (ret1 != 1) return false;
    int ret2 = i2c_read_blocking(i2c_, addr, value, 1, false);
    return ret2 == 1;
}

bool HAL_I2C_RP2350::write_read(uint8_t addr,
                                const uint8_t* wdata, size_t wlen,
                                uint8_t* rdata, size_t rlen) {
    if ((wlen > 0 && !wdata) || (rlen > 0 && !rdata)) return false;

    if (wlen > 0) {
        int ret1 = i2c_write_blocking(i2c_, addr, wdata, (int)wlen, true);
        if (ret1 != (int)wlen) return false;
    }
    if (rlen > 0) {
        int ret2 = i2c_read_blocking(i2c_, addr, rdata, (int)rlen, false);
        return ret2 == (int)rlen;
    }
    return true;
}
