#ifndef ST7796_DRIVER_HPP
#define ST7796_DRIVER_HPP

#include "drivers/display/interface/display_interface.hpp"
#include "hal/interfaces/hal_spi_interface.hpp"
#include <stdint.h>

/**
 * @brief Driver para Display ST7796 (4.0" IPS SPI)
 */
class ST7796Driver : public DisplayInterface {
private:
    HAL_SPI_Interface* hal_spi_;
    uint16_t width_;
    uint16_t height_;
    uint8_t rotation_;

    void setAddressWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

     // ST7796 Commands
    static constexpr uint8_t CMD_NOP = 0x00;
    static constexpr uint8_t CMD_SWRESET = 0x01;
    static constexpr uint8_t CMD_SLPIN = 0x10;
    static constexpr uint8_t CMD_SLPOUT = 0x11;
    static constexpr uint8_t CMD_INVOFF = 0x20;
    static constexpr uint8_t CMD_INVON = 0x21;
    static constexpr uint8_t CMD_DISPOFF = 0x28;
    static constexpr uint8_t CMD_DISPON = 0x29;
    static constexpr uint8_t CMD_CASET = 0x2A;
    static constexpr uint8_t CMD_RASET = 0x2B;
    static constexpr uint8_t CMD_RAMWR = 0x2C;
    static constexpr uint8_t CMD_MADCTL = 0x36;
    static constexpr uint8_t CMD_COLMOD = 0x3A;

    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void writeData(const uint8_t* data, size_t len);

public:
    ST7796Driver(HAL_SPI_Interface* hal_spi, 
                 uint16_t width = 480, 
                 uint16_t height = 320);

    bool init() override;
    uint16_t getWidth() const override { return width_; }
    uint16_t getHeight() const override { return height_; }
    
    void drawPixels(uint16_t x1, uint16_t y1, 
                   uint16_t x2, uint16_t y2, 
                   const uint16_t* color_data) override;
    
    void fillRect(uint16_t x1, uint16_t y1, 
                 uint16_t x2, uint16_t y2, 
                 uint16_t color) override;
    
    void setBacklight(uint8_t brightness) override;
    void setPower(bool state) override;
    void setRotation(uint8_t rotation) override;
};

#endif // ST7796_DRIVER_HPP