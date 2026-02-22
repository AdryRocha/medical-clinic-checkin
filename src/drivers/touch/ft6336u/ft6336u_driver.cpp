#include "ft6336u_driver.hpp"
#include <stdio.h>
#include "pico/stdlib.h"

FT6336U_Driver::FT6336U_Driver(HAL_I2C_Interface* i2c, 
                               uint8_t address,
                               uint16_t width, 
                               uint16_t height,
                               uint8_t pin_rst,
                               uint8_t pin_int)
    : i2c_(i2c),
      i2c_address_(address),
      screen_width_(width),
      screen_height_(height),
      pin_rst_(pin_rst),
      pin_int_(pin_int),
      initialized_(false) {
}

bool FT6336U_Driver::readRegister(uint8_t reg, uint8_t* value) {
    if (i2c_ == nullptr || value == nullptr) {
        return false;
    }
    return i2c_->writeRead(i2c_address_, &reg, 1, value, 1);
}

bool FT6336U_Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t length) {
    if (i2c_ == nullptr || buffer == nullptr || length == 0) {
        return false;
    }
    return i2c_->writeRead(i2c_address_, &reg, 1, buffer, length);
}

void FT6336U_Driver::reset() {
    if (pin_rst_ == 0xFF) {
        return;
    }

    gpio_init(pin_rst_);
    gpio_set_dir(pin_rst_, GPIO_OUT);
    
    // Reset sequence: HIGH -> LOW (20ms) -> HIGH -> wait 300ms
    gpio_put(pin_rst_, 1);
    sleep_ms(10);
    gpio_put(pin_rst_, 0);
    sleep_ms(20);
    gpio_put(pin_rst_, 1);
    sleep_ms(300);
}

bool FT6336U_Driver::init() {
    if (i2c_ == nullptr) {
        return false;
    }

    if (pin_int_ != 0xFF) {
        gpio_init(pin_int_);
        gpio_set_dir(pin_int_, GPIO_IN);
        gpio_pull_up(pin_int_);
    }

    reset();

    uint8_t chip_id = 0;
    if (!readRegister(REG_CHIP_ID, &chip_id)) {
        return false;
    }

    initialized_ = true;
    return true;
}

bool FT6336U_Driver::isTouched() {
    if (!initialized_) {
        return false;
    }

    uint8_t td_status = 0;
    if (!readRegister(REG_TD_STATUS, &td_status)) {
        return false;
    }

    // TD_STATUS bits [3:0] contain the number of touch points detected
    uint8_t touch_count = td_status & 0x0F;
    return touch_count > 0;
}

bool FT6336U_Driver::readTouch(TouchPoint* point) {
    if (!initialized_ || point == nullptr) {
        return false;
    }

    uint8_t td_status = 0;
    if (!readRegister(REG_TD_STATUS, &td_status)) {
        point->valid = false;
        return false;
    }

    uint8_t touch_count = td_status & 0x0F;
    
    if (touch_count == 0) {
        point->valid = false;
        return true;
    }

    // Read touch point data (6 bytes: XH, XL, YH, YL, and event flags)
    uint8_t data[4];
    if (!readRegisters(REG_TOUCH1_XH, data, 4)) {
        point->valid = false;
        return false;
    }

    // Parse touch data
    // XH: [7:6] = event flag, [3:0] = X[11:8]
    // XL: [7:0] = X[7:0]
    // YH: [7:6] = touch ID, [3:0] = Y[11:8]
    // YL: [7:0] = Y[7:0]
    
    point->event = (data[0] >> 6) & 0x03;
    point->x = ((data[0] & 0x0F) << 8) | data[1];
    point->y = ((data[2] & 0x0F) << 8) | data[3];
    
    if (point->x >= screen_width_) {
        point->x = screen_width_ - 1;
    }
    if (point->y >= screen_height_) {
        point->y = screen_height_ - 1;
    }
    
    point->valid = true;
    return true;
}

uint8_t FT6336U_Driver::getChipID() {
    uint8_t chip_id = 0;
    readRegister(REG_CHIP_ID, &chip_id);
    return chip_id;
}

uint8_t FT6336U_Driver::getFirmwareVersion() {
    uint8_t fw_ver = 0;
    readRegister(REG_FW_VER, &fw_ver);
    return fw_ver;
}
