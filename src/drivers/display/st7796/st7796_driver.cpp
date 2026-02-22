#include "pin_config.hpp"
#include "st7796_driver.hpp"
#include "pico/stdlib.h"
#include <stdio.h>
#include "pin_config.hpp"

// --- DEFINIÇÕES DE COMANDOS ST7796 ---
// Definidas aqui para uso local no driver
#define CMD_SWRESET 0x01
#define CMD_SLPIN   0x10
#define CMD_SLPOUT  0x11
#define CMD_NORON   0x13
#define CMD_INVOFF  0x20
#define CMD_INVON   0x21
#define CMD_DISPOFF 0x28
#define CMD_DISPON  0x29
#define CMD_CASET   0x2A
#define CMD_RASET   0x2B
#define CMD_RAMWR   0x2C
#define CMD_MADCTL  0x36
#define CMD_COLMOD  0x3A

// Comandos Estendidos (Power/Gamma) - Magic Init
#define CMD_CSCON   0xF0 
#define CMD_INVCTR  0xB4 
#define CMD_DFUNCTR 0xB6 
#define CMD_PWCTR2  0xC1 
#define CMD_PWCTR3  0xC2 
#define CMD_VMCTR   0xC5 
#define CMD_DOCA    0xE8 
#define CMD_GMCTRP1 0xE0 
#define CMD_GMCTRN1 0xE1 

// --- IMPLEMENTAÇÃO DA CLASSE ---

ST7796Driver::ST7796Driver(HAL_SPI_Interface* hal_spi, 
                           uint16_t width, 
                           uint16_t height)
    : hal_spi_(hal_spi),
      width_(width),
      height_(height),
      rotation_(0) {
}

// Helpers de Baixo Nível
void ST7796Driver::writeCommand(uint8_t cmd) {
    hal_spi_->setDC(false); // Command Mode
    hal_spi_->setCS(true);  // Select (Active Low)
    hal_spi_->write(&cmd, 1);
    hal_spi_->setCS(false); // Deselect
}

void ST7796Driver::writeData(uint8_t data) {
    hal_spi_->setDC(true);  // Data Mode
    hal_spi_->setCS(true);  // Select
    hal_spi_->write(&data, 1);
    hal_spi_->setCS(false); // Deselect
}

void ST7796Driver::writeData(const uint8_t* data, size_t len) {
    hal_spi_->setDC(true);  // Data Mode
    hal_spi_->setCS(true);  // Select
    hal_spi_->write(data, len);
    hal_spi_->setCS(false); // Deselect
}

bool ST7796Driver::init() {
    if (!hal_spi_->init(10 * 1000 * 1000)) {  // Reduza pra 10MHz inicial (aumente depois se ok)
        return false;
    }
    
    hal_spi_->reset();

    // Sequência full do datasheet ST7796S
    writeCommand(CMD_SWRESET);  // 0x01
    sleep_ms(120);  // Delay >=120ms

    writeCommand(CMD_SLPOUT);   // 0x11
    sleep_ms(120);  // Delay >=120ms

    // Power Control
    writeCommand(CMD_CSCON);    // 0xF0 - Enable command
    writeData(0xC3);

    writeCommand(CMD_PWCTR2);   // 0xC1
    writeData(0x41);  // GVDD=4.3V

    writeCommand(CMD_PWCTR3);   // 0xC2
    writeData(0x0D);  // OPON/Ionic/Follower

    writeCommand(CMD_VMCTR);    // 0xC5
    writeData(0x30);  // VCOM= -1.0V (ajuste se washed out)

    // Gamma Positive
    writeCommand(CMD_GMCTRP1);  // 0xE0
    writeData(0xD0); writeData(0x00); writeData(0x02); writeData(0x07);
    writeData(0x0A); writeData(0x28); writeData(0x32); writeData(0x44);
    writeData(0x42); writeData(0x06); writeData(0x0E); writeData(0x12);
    writeData(0x14); writeData(0x17); writeData(0x1B);

    // Gamma Negative
    writeCommand(CMD_GMCTRN1);  // 0xE1
    writeData(0xD0); writeData(0x00); writeData(0x02); writeData(0x07);
    writeData(0x0A); writeData(0x28); writeData(0x31); writeData(0x54);
    writeData(0x47); writeData(0x0E); writeData(0x0C); writeData(0x17);
    writeData(0x13); writeData(0x1B); writeData(0x1C);

    // Inversion Off (evita invertido/chuvisco)
    writeCommand(CMD_INVOFF);   // 0x20 (tente 0x21 se cores invertidas)
    // MADCTL pra RGB (não BGR) e orientação default
    writeCommand(CMD_MADCTL);   // 0x36
    writeData(0x28);  // MX=0, MY=0, MV=0, ML=0, RGB=0 (ajuste pra rotação)

    // Color Mode RGB565
    writeCommand(CMD_COLMOD);   // 0x3A
    writeData(0x55);  // 16-bit

    // Display On
    writeCommand(CMD_DISPON);   // 0x29
    sleep_ms(5);  // Delay >=5ms

    // Clear screen pra remover garbage/chuvisco inicial
    fillRect(0, 0, width_ - 1, height_ - 1, 0x0000);  // Black
    sleep_ms(200);

    return true;
}

void ST7796Driver::drawPixels(uint16_t x1, uint16_t y1, 
                              uint16_t x2, uint16_t y2, 
                              const uint16_t* color_data) {
    setAddressWindow(x1, y1, x2, y2);
    
    size_t len = (x2 - x1 + 1) * (y2 - y1 + 1);
    
    hal_spi_->setCS(true);
    hal_spi_->setDC(true);
    hal_spi_->write((const uint8_t*)color_data, len * 2);
    hal_spi_->setCS(false);
}

void ST7796Driver::fillRect(uint16_t x1, uint16_t y1, 
                            uint16_t x2, uint16_t y2, 
                            uint16_t color) {
    setAddressWindow(x1, y1, x2, y2);
    
    size_t pixels = (x2 - x1 + 1) * (y2 - y1 + 1);
    
    uint8_t color_bytes[2] = {
        (uint8_t)(color >> 8),
        (uint8_t)(color & 0xFF)
    };
    
    hal_spi_->setCS(true);
    hal_spi_->setDC(true);
    
    for (size_t i = 0; i < pixels; i++) {
        hal_spi_->write(color_bytes, 2);
    }
    
    hal_spi_->setCS(false);
}

void ST7796Driver::setBacklight(uint8_t brightness) {
    // TODO LATER: Implement PWM backlight control via HAL
}

void ST7796Driver::setPower(bool state) {
    if (state) {
        writeCommand(CMD_SLPOUT);
        sleep_ms(120);
        writeCommand(CMD_DISPON);
    } else {
        writeCommand(CMD_DISPOFF);
        writeCommand(CMD_SLPIN);
    }
}

void ST7796Driver::setRotation(uint8_t rotation) {
    rotation_ = rotation % 4;
    
    uint8_t madctl = 0x48;
    
    switch (rotation_) {
        case 0: // Portrait
            madctl = 0x48;
            width_ = 320;
            height_ = 480;
            break;
        case 1: // Landscape
            madctl = 0x28;
            width_ = 480;
            height_ = 320;
            break;
        case 2: // Portrait inverted
            madctl = 0x88;
            width_ = 320;
            height_ = 480;
            break;
        case 3: // Landscape inverted
            madctl = 0xE8;
            width_ = 480;
            height_ = 320;
            break;
    }
    
    writeCommand(CMD_MADCTL);
    writeData(madctl);
}

// Implementação da função
void ST7796Driver::setAddressWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    // CMD_CASET: Define colunas (X)
    writeCommand(CMD_CASET);  // 0x2A
    writeData(x1 >> 8);       // MSB
    writeData(x1 & 0xFF);     // LSB
    writeData(x2 >> 8);       // MSB
    writeData(x2 & 0xFF);     // LSB

    // CMD_RASET: Define linhas (Y)
    writeCommand(CMD_RASET);  // 0x2B
    writeData(y1 >> 8);       // MSB
    writeData(y1 & 0xFF);     // LSB
    writeData(y2 >> 8);       // MSB
    writeData(y2 & 0xFF);     // LSB

    // CMD_RAMWR: Inicia escrita na RAM (pixels)
    writeCommand(CMD_RAMWR);  // 0x2C
}