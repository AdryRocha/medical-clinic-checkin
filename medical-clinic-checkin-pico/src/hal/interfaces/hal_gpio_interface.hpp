#pragma once
#include <cstdint>

// Enumerações essenciais - Fora da classe
enum class PinDirection { Input, Output };
enum class PinFunction { GPIO, SPI, I2C, UART, PWM }; // Adicionei GPIO que faltava no seu erro

class HAL_GPIO_Interface {
public:
    virtual ~HAL_GPIO_Interface() = default;

    virtual void init(uint32_t pin, PinDirection dir) = 0;
    virtual void setFunction(uint32_t pin, PinFunction func) = 0;
    virtual void write(uint32_t pin, bool value) = 0;
    virtual bool read(uint32_t pin) = 0;
    virtual void toggle(uint32_t pin) = 0;

    // Métodos opcionais (pode deixar vazio na implementação se não usar)
    virtual void setFunction(uint32_t pin, uint8_t func) {} 
    virtual void pullUp(uint32_t pin) {}
    virtual void pullDown(uint32_t pin) {}

    // Métodos de compatibilidade (para não quebrar drivers antigos)
    void setOutput(uint32_t pin, bool value = false) {
        init(pin, PinDirection::Output);
        write(pin, value);
    }
    
    void setInput(uint32_t pin) {
        init(pin, PinDirection::Input);
    }
};