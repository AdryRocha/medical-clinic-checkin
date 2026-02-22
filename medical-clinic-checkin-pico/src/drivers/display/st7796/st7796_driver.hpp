#ifndef ST7796_DRIVER_HPP
#define ST7796_DRIVER_HPP

#include "drivers/display/interface/display_interface.hpp"
#include "hal/interfaces/hal_spi_interface.hpp"
#include "hal/interfaces/hal_gpio_interface.hpp"
#include <stdint.h>
#include <stddef.h>

class ST7796Driver : public DisplayInterface {
public:
    ST7796Driver(HAL_SPI_Interface* spi,
                 HAL_GPIO_Interface* gpio,
                 uint8_t cs_pin,
                 uint8_t dc_pin,
                 uint8_t rst_pin,
                 uint8_t bl_pin,
                 uint16_t width,
                 uint16_t height);

    bool init() override;

    uint16_t getWidth()  const override { return width_; }
    uint16_t getHeight() const override { return height_; }

    void drawPixels(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const uint16_t* color_data) override;
    void fillRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) override;

    void fillScreen(uint16_t color);

    void setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

    void setBacklight(uint8_t brightness) override;     // 0..100 (PWM)
    void setPower(bool state) override { (void)state; } // stub ok
    void setRotation(uint8_t rotation) override;        // 0..3

private:
    void reset();
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void writeData(const uint8_t* data, size_t len);
    void writeColorBlock(uint16_t color, uint32_t count); // stub

    HAL_SPI_Interface* spi_;
    HAL_GPIO_Interface* gpio_;
    uint8_t cs_pin_, dc_pin_, rst_pin_, bl_pin_;
    uint16_t width_, height_;
    uint8_t rotation_;
};

#endif
