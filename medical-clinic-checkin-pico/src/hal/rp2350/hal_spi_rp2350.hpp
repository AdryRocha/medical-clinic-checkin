#ifndef HAL_SPI_RP2350_HPP
#define HAL_SPI_RP2350_HPP

#include "hal/interfaces/hal_spi_interface.hpp"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

class HAL_SPI_RP2350 : public HAL_SPI_Interface {
public:
    // Construtor atualizado para 7 argumentos (SPI + 6 Pinos)
    HAL_SPI_RP2350(spi_inst_t* spi_inst, 
                   uint8_t pin_sck, uint8_t pin_tx, uint8_t pin_rx, 
                   uint8_t pin_cs, uint8_t pin_dc, uint8_t pin_rst);

    // Implementação da Interface
    bool init(uint32_t baud_rate) override;
    bool write(const uint8_t* data, size_t len) override;
    bool transfer(const uint8_t* tx, uint8_t* rx, size_t len) override;
    
    // Método específico (sem override, pois não existe na base)
    bool read(uint8_t* data, size_t len);

    // Métodos de controle manual
    void setCS(bool state);
    void setDC(bool state);
    void reset();

private:
    spi_inst_t* spi_inst_;
    uint8_t pin_sck_;
    uint8_t pin_tx_;
    uint8_t pin_rx_;
    uint8_t pin_cs_;
    uint8_t pin_dc_;
    uint8_t pin_rst_;
};

#endif