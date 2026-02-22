#include "drivers/display/st7796/st7796_driver.hpp"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <cstring>
#include <lvgl.h>
#include <algorithm>
#include "pinmap.hpp"
#include "display_config.hpp"
#include "services/logger_service.hpp"

// --- DEFINIÇÕES DE COMANDOS ST7796 ---
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
#define CMD_CSCON   0xF0

// Comandos Estendidos
#define CMD_PWR1    0xC0 // Power Control 1
#define CMD_PWR2    0xC1 // Power Control 2
#define CMD_PWR3    0xC2 // Power Control 3
#define CMD_VCOM    0xC5 // VCOM Control
#define CMD_PGC     0xE0 // Positive Gamma Control
#define CMD_NGC     0xE1 // Negative Gamma Control
#define CMD_DOCA    0xE8 // Display Output Ctrl
#define CMD_INVCTR  0xB4 // Display Inversion Control (Era DICTR)
#define CMD_DFUNCTR 0xB6 // Display Function Control 

// MADCTL bits
static constexpr uint8_t MADCTL_MY  = 0x80;
static constexpr uint8_t MADCTL_MX  = 0x40;
static constexpr uint8_t MADCTL_MV  = 0x20;
static constexpr uint8_t MADCTL_BGR = 0x08; // Flag para ordem de cores

// Rotations validados no protótipo (ST7796 320x480)
static constexpr uint8_t ST7796_ROTATION_0   = 0x48; // Portrait (320x480) - MX + BGR
static constexpr uint8_t ST7796_ROTATION_90  = 0x28; // Landscape (480x320) - MV + BGR
static constexpr uint8_t ST7796_ROTATION_180 = 0x88; // Portrait invertido (320x480) - MY + BGR
static constexpr uint8_t ST7796_ROTATION_270 = 0xE8; // Landscape invertido (480x320) - MY + MX + MV + BGR

static uint8_t rotation_to_madctl( uint8_t rotation )
{
    switch( rotation % 4 )
    {
        case 0: return ST7796_ROTATION_0;
        case 1: return ST7796_ROTATION_90;
        case 2: return ST7796_ROTATION_180;
        case 3: return ST7796_ROTATION_270;
    }
    return ST7796_ROTATION_0;
}

ST7796Driver::ST7796Driver(HAL_SPI_Interface* spi, HAL_GPIO_Interface* gpio, 
                           uint8_t cs_pin, uint8_t dc_pin, uint8_t rst_pin, uint8_t bl_pin, 
                           uint16_t width, uint16_t height)
    : spi_(spi), gpio_(gpio), 
      cs_pin_(cs_pin), dc_pin_(dc_pin), rst_pin_(rst_pin), bl_pin_(bl_pin),
      width_(width), height_(height) 
{
    // Usando 'Output' (conforme definido no seu HAL) em vez de 'OUTPUT'
    gpio_->init(cs_pin_, PinDirection::Output);
    gpio_->init(dc_pin_, PinDirection::Output);
    gpio_->init(rst_pin_, PinDirection::Output);
    
    // Inicia CS e RST como High (Desabilitado/Inativo)
    gpio_->write(cs_pin_, true); 
    gpio_->write(rst_pin_, true);

    // Configura Backlight se o pino for válido
    if (bl_pin_ < 255) { 
        gpio_->init(bl_pin_, PinDirection::Output);
        gpio_->write(bl_pin_, false); // Começa desligado
    }
}

bool ST7796Driver::init() {
    Logger::info("[DRIVER] Iniciando sequencia de reset ST7796 (prototipo_3)");

    // Reset de hardware
    gpio_->write(rst_pin_, 0);
    sleep_ms(20);
    gpio_->write(rst_pin_, 1);
    sleep_ms(150);

    // Reset de software + saída do sleep
    writeCommand(CMD_SWRESET);
    sleep_ms(20);
    writeCommand(CMD_SLPOUT);
    sleep_ms(120);

    // Formato de cor RGB565
    writeCommand(0x3A); writeData(0x55);

    // Orientação validada no protótipo
    setRotation(DISP_ROTATION);

    // Range completo da tela
    setAddressWindow(0, 0, width_ - 1, height_ - 1);

    // Inversão off, modo normal e liga display
    writeCommand(0x20); // INVOFF
    sleep_ms(10);
    writeCommand(0x13); // NORON
    sleep_ms(10);
    writeCommand(0x29); // DISPON
    sleep_ms(50);

    Logger::info("[DRIVER] ST7796 inicializado (sequencia protótipo)");

    setBacklight(100);
    return true;
}

    void ST7796Driver::setBacklight(uint8_t brightness) {
    if (bl_pin_ >= 255) return;
    if (brightness > 100) brightness = 100;

    gpio_set_function(bl_pin_, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(bl_pin_);
    uint chan  = pwm_gpio_to_channel(bl_pin_);

    pwm_set_wrap(slice, 255);
    uint16_t level = (brightness * 255u) / 100u;
    pwm_set_chan_level(slice, chan, level);
    pwm_set_enabled(slice, true);
}

void ST7796Driver::setRotation(uint8_t m) {
    writeCommand(CMD_MADCTL);
    rotation_ = m % 4;

    const uint16_t _native_w = 320;
    const uint16_t _native_h = 480;

    const uint8_t madctl = rotation_to_madctl( rotation_ );
    writeData(madctl);

    width_ = (rotation_ % 2 == 0) ? _native_w : _native_h;
    height_ = (rotation_ % 2 == 0) ? _native_h : _native_w;
}

void ST7796Driver::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    setAddressWindow(x, y, x + w - 1, y + h - 1);
    uint32_t count = (uint32_t)w * h;
    
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    uint8_t line_buf[256]; // Buffer maior para performance
    for (int i = 0; i < 128; i++) { line_buf[i * 2] = hi; line_buf[i * 2 + 1] = lo; }
    
    gpio_->write(dc_pin_, true);
    gpio_->write(cs_pin_, false);
    
    uint32_t sent = 0;
    while (sent < count) {
        uint32_t chunk = (count - sent > 128) ? 128 : (count - sent);
        spi_->write(line_buf, chunk * 2);
        sent += chunk;
    }
    gpio_->write(cs_pin_, true);
}

void ST7796Driver::fillScreen(uint16_t color) {
    fillRect(0, 0, width_, height_, color);
}

void ST7796Driver::writeColorBlock(uint16_t color, uint32_t count) { (void)color; (void)count; }

void ST7796Driver::drawPixels(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data) {
    if((x >= width_) || (y >= height_)) return;
    setAddressWindow(x, y, x + w - 1, y + h - 1);
    
    gpio_->write(dc_pin_, true);
    gpio_->write(cs_pin_, false);
    spi_->write((const uint8_t*)data, w * h * 2);
    gpio_->write(cs_pin_, true);
}

void ST7796Driver::setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    /* MADCTL already applies MV/MX/MY inside the panel. Do not swap here; 
       use the logical coordinates in the current rotation. */

    writeCommand(CMD_CASET);
    uint8_t x_data[] = { (uint8_t)(x0 >> 8), (uint8_t)(x0 & 0xFF), (uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF) };
    writeData(x_data, 4);

    writeCommand(CMD_RASET);
    uint8_t y_data[] = { (uint8_t)(y0 >> 8), (uint8_t)(y0 & 0xFF), (uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF) };
    writeData(y_data, 4);

    writeCommand(CMD_RAMWR);
}

void ST7796Driver::writeCommand(uint8_t cmd) {
    gpio_->write(dc_pin_, false); 
    gpio_->write(cs_pin_, false); 
    spi_->write(&cmd, 1);
    gpio_->write(cs_pin_, true);  
}

void ST7796Driver::writeData(uint8_t data) {
    gpio_->write(dc_pin_, true);  
    gpio_->write(cs_pin_, false); 
    spi_->write(&data, 1);
    gpio_->write(cs_pin_, true);  
}

void ST7796Driver::writeData(const uint8_t* data, size_t len) {
    sleep_us(5);               // GAP 2: Delay para estabilização do pino DC
    gpio_->write(dc_pin_, 1);  // DC HIGH = Dados
    gpio_->write(cs_pin_, 0);  // Seleciona display
    spi_->write(data, len);    // Envia via HAL
    gpio_->write(cs_pin_, 1);  // Deseleciona
}