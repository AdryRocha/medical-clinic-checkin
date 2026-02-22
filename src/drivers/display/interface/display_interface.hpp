#ifndef DISPLAY_INTERFACE_HPP
#define DISPLAY_INTERFACE_HPP

#include <stdint.h>


/**
 * @brief Classe Base Abstrata para Drivers de Display
 */
class DisplayInterface {
public:
    virtual ~DisplayInterface() = default;

    // Funções essenciais requeridas pelo adaptador LVGL
    virtual bool init() = 0;
    virtual uint16_t getWidth() const = 0;
    virtual uint16_t getHeight() const = 0;
    virtual void drawPixels(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const uint16_t* color_data) = 0;

    // Funções opcionais
    virtual void fillRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) = 0;
    virtual void setBacklight(uint8_t brightness) = 0;
    virtual void setPower(bool state) = 0;
    virtual void setRotation(uint8_t rotation) = 0;
}; 


#endif // DISPLAY_INTERFACE_HPP
