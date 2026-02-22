#include "hal/rp2350/hal_spi_rp2350.hpp"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

// Construtor: Inicializa as variáveis membro com os nomes corretos
HAL_SPI_RP2350::HAL_SPI_RP2350(spi_inst_t* spi_inst, 
                               uint8_t pin_sck, uint8_t pin_tx, uint8_t pin_rx, 
                               uint8_t pin_cs, uint8_t pin_dc, uint8_t pin_rst)
    : spi_inst_(spi_inst), 
      pin_sck_(pin_sck), pin_tx_(pin_tx), pin_rx_(pin_rx), 
      pin_cs_(pin_cs), pin_dc_(pin_dc), pin_rst_(pin_rst) 
{
}

bool HAL_SPI_RP2350::init(uint32_t baud_rate) {
    // CORREÇÃO: Usando 'spi_inst_' em vez de 'spi_'
    spi_init(spi_inst_, baud_rate);
    
    // CORREÇÃO: Usando prefixo 'pin_' conforme o header
    gpio_set_function(pin_sck_, GPIO_FUNC_SPI);
    gpio_set_function(pin_tx_,  GPIO_FUNC_SPI); // MOSI
    gpio_set_function(pin_rx_,  GPIO_FUNC_SPI); // MISO

    // Configura CS (Chip Select)
    gpio_init(pin_cs_);
    gpio_set_dir(pin_cs_, GPIO_OUT);
    gpio_put(pin_cs_, 1); // Deselecionado (High)

    // Configura DC (Data/Command) - Se definido
    if (pin_dc_ != 0xFF) {
        gpio_init(pin_dc_);
        gpio_set_dir(pin_dc_, GPIO_OUT);
        gpio_put(pin_dc_, 1); // Dados por padrão
    }

    // Configura Reset - Se definido
    if (pin_rst_ != 0xFF) {
        gpio_init(pin_rst_);
        gpio_set_dir(pin_rst_, GPIO_OUT);
        gpio_put(pin_rst_, 1); // Sem reset
    }

    return true;
}

bool HAL_SPI_RP2350::write(const uint8_t* data, size_t len) {
    spi_write_blocking(spi_inst_, data, len);
    return true;
}

// Implementação do read (sem override no header, mas implementado aqui)
bool HAL_SPI_RP2350::read(uint8_t* data, size_t len) {
    spi_read_blocking(spi_inst_, 0, data, len);
    return true;
}

bool HAL_SPI_RP2350::transfer(const uint8_t* tx, uint8_t* rx, size_t len) {
    spi_write_read_blocking(spi_inst_, tx, rx, len);
    return true;
}

void HAL_SPI_RP2350::setCS(bool state) {
    // state = true -> Active (Low)
    gpio_put(pin_cs_, !state); 
}

void HAL_SPI_RP2350::setDC(bool state) {
    // state = true -> Data (High), false -> Command (Low)
    if (pin_dc_ != 0xFF) gpio_put(pin_dc_, state);
}

void HAL_SPI_RP2350::reset() {
    if (pin_rst_ != 0xFF) {
        gpio_put(pin_rst_, 0); // Reset Ativo
        sleep_ms(20);
        gpio_put(pin_rst_, 1); // Reset Inativo
        sleep_ms(150);
    }
}