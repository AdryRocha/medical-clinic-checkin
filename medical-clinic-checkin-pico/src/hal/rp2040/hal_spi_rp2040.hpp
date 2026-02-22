#ifndef HAL_SPI_RP2040_HPP
#define HAL_SPI_RP2040_HPP

#include "hal/interfaces/hal_spi_interface.hpp"
#include "hardware/spi.h"
#include "hardware/gpio.h"

/**
 * @brief RP2040-specific SPI HAL implementation
 */
class HAL_SPI_RP2040 : public HAL_SPI_Interface {
private:
    spi_inst_t* spi_instance_;
    uint8_t pin_mosi_;
    uint8_t pin_miso_;
    uint8_t pin_sck_;
    uint8_t pin_cs_;
    uint8_t pin_dc_;
    uint8_t pin_rst_;

public:
    /**
     * @brief Construct SPI HAL for RP2040
     * @param spi_instance SPI peripheral (spi0 or spi1)
     * @param pin_mosi MOSI pin number
     * @param pin_miso MISO pin number (optional, set to 0xFF if not used)
     * @param pin_sck SCK pin number
     * @param pin_cs Chip Select pin number
     * @param pin_dc Data/Command pin number
     * @param pin_rst Reset pin number
     */
    HAL_SPI_RP2040(spi_inst_t* spi_instance, 
                   uint8_t pin_mosi, 
                   uint8_t pin_miso,
                   uint8_t pin_sck, 
                   uint8_t pin_cs, 
                   uint8_t pin_dc, 
                   uint8_t pin_rst);

    bool init(uint32_t baudrate) override;
    size_t write(const uint8_t* data, size_t len) override;
    size_t read(uint8_t* data, size_t len) override;
    void setCS(bool state) override;
    void setDC(bool state) override;
    void reset() override;
};

#endif // HAL_SPI_RP2040_HPP