#ifndef FT6336U_DRIVER_HPP
#define FT6336U_DRIVER_HPP

#include "drivers/touch/interface/touch_interface.hpp"
#include "hal/interfaces/hal_i2c_interface.hpp"

/**
 * @brief Driver for FT6336U capacitive touch controller
 * 
 * This driver supports the FocalTech FT6336U touch controller commonly
 * used in capacitive touchscreens. It communicates via I2C.
 */
class FT6336U_Driver : public TouchInterface {
private:
    HAL_I2C_Interface* i2c_;
    uint8_t i2c_address_;
    uint16_t screen_width_;
    uint16_t screen_height_;
    uint8_t pin_rst_;
    uint8_t pin_int_;
    bool initialized_;

    // FT6336U register addresses
    static constexpr uint8_t REG_MODE = 0x00;
    static constexpr uint8_t REG_TD_STATUS = 0x02;
    static constexpr uint8_t REG_TOUCH1_XH = 0x03;
    static constexpr uint8_t REG_TOUCH1_XL = 0x04;
    static constexpr uint8_t REG_TOUCH1_YH = 0x05;
    static constexpr uint8_t REG_TOUCH1_YL = 0x06;
    static constexpr uint8_t REG_CHIP_ID = 0xA3;
    static constexpr uint8_t REG_FW_VER = 0xA6;

    // Expected chip ID for FT6336U
    static constexpr uint8_t CHIP_ID_FT6336U = 0x64;

    /**
     * @brief Read a single register from the touch controller
     * @param reg Register address
     * @param value Pointer to store read value
     * @return true if read was successful
     */
    bool readRegister(uint8_t reg, uint8_t* value);

    /**
     * @brief Read multiple registers from the touch controller
     * @param reg Starting register address
     * @param buffer Buffer to store read values
     * @param length Number of bytes to read
     * @return true if read was successful
     */
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t length);

public:
    /**
     * @brief Construct FT6336U driver
     * @param i2c Pointer to I2C HAL interface
     * @param address I2C device address (default 0x38)
     * @param width Screen width in pixels
     * @param height Screen height in pixels
     * @param pin_rst Reset pin GPIO number (0xFF = not used)
     * @param pin_int Interrupt pin GPIO number (0xFF = not used)
     */
    FT6336U_Driver(HAL_I2C_Interface* i2c, 
                   uint8_t address = 0x38,
                   uint16_t width = 480, 
                   uint16_t height = 320,
                   uint8_t pin_rst = 0xFF,
                   uint8_t pin_int = 0xFF);

    /**
     * @brief Perform hardware reset of touch controller
     */
    void reset();

    bool init() override;
    bool readTouch(TouchPoint* point) override;
    bool isTouched() override;
    uint8_t getMaxTouchPoints() const override { return 2; }

    /**
     * @brief Get the chip ID
     * @return Chip ID value
     */
    uint8_t getChipID();

    /**
     * @brief Get the firmware version
     * @return Firmware version
     */
    uint8_t getFirmwareVersion();
};

#endif // FT6336U_DRIVER_HPP
