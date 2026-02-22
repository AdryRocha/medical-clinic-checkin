#include "hal_spi_rp2040.hpp"
#include "pico/stdlib.h"
#include "hardware/spi.h"

HAL_SPI_RP2040::HAL_SPI_RP2040(spi_inst_t* spi_instance, 
                               uint8_t pin_mosi, 
                               uint8_t pin_miso,
                               uint8_t pin_sck, 
                               uint8_t pin_cs, 
                               uint8_t pin_dc, 
                               uint8_t pin_rst)
    : spi_instance_(spi_instance),
      pin_mosi_(pin_mosi),
      pin_miso_(pin_miso),
      pin_sck_(pin_sck),
      pin_cs_(pin_cs),
      pin_dc_(pin_dc),
      pin_rst_(pin_rst) {
}

bool HAL_SPI_RP2040::init(uint32_t baudrate) {
    // Inicializa o SPI com a velocidade solicitada
    spi_init(spi_instance_, baudrate);
    
    // CORREÇÃO: Forçar formato SPI correto para displays TFT (Mode 0)
    // 8 data bits, CPOL 0, CPHA 0, MSB First
    spi_set_format(spi_instance_, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    
    gpio_set_function(pin_mosi_, GPIO_FUNC_SPI);
    gpio_set_function(pin_sck_, GPIO_FUNC_SPI);
    
    if (pin_miso_ != 0xFF) {
        gpio_set_function(pin_miso_, GPIO_FUNC_SPI);
    }
    
    gpio_init(pin_cs_);
    gpio_set_dir(pin_cs_, GPIO_OUT);
    gpio_put(pin_cs_, 1);
    
    gpio_init(pin_dc_);
    gpio_set_dir(pin_dc_, GPIO_OUT);
    gpio_put(pin_dc_, 1); // Default High
    
    gpio_init(pin_rst_);
    gpio_set_dir(pin_rst_, GPIO_OUT);
    gpio_put(pin_rst_, 1);
    
    return true;
}

size_t HAL_SPI_RP2040::write(const uint8_t* data, size_t len) {
    return spi_write_blocking(spi_instance_, data, len);
}

size_t HAL_SPI_RP2040::read(uint8_t* data, size_t len) {
    return spi_read_blocking(spi_instance_, 0, data, len);
}

void HAL_SPI_RP2040::setCS(bool state) {
    gpio_put(pin_cs_, state ? 0 : 1); // Active Low
}

void HAL_SPI_RP2040::setDC(bool state) {
    gpio_put(pin_dc_, state ? 1 : 0); // Data=1, Cmd=0
}

void HAL_SPI_RP2040::reset() {
    // Sequência de Reset Hardware padrão
    gpio_put(pin_rst_, 1);
    sleep_ms(50);
    gpio_put(pin_rst_, 0);
    sleep_ms(50); // Reset Low por 50ms
    gpio_put(pin_rst_, 1);
    sleep_ms(150); // Aguarda boot do controller
}