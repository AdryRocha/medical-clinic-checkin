#pragma once

#include <cstdint>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "hal/interfaces/hal_gpio_interface.hpp"

class HAL_GPIO_RP2350 : public HAL_GPIO_Interface {
public:
    HAL_GPIO_RP2350() = default;
    ~HAL_GPIO_RP2350() override = default;

    void init(uint32_t pin, PinDirection dir) override {
        gpio_init(pin);
        gpio_set_dir(pin, (dir == PinDirection::Output) ? GPIO_OUT : GPIO_IN);
        if (dir == PinDirection::Output) {
            gpio_put(pin, 0);
        }
    }

    void setFunction(uint32_t pin, PinFunction func) override {
        gpio_function_t f = GPIO_FUNC_SIO;
        switch (func) {
            case PinFunction::GPIO: f = GPIO_FUNC_SIO; break;
            case PinFunction::SPI:  f = GPIO_FUNC_SPI; break;
            case PinFunction::I2C:  f = GPIO_FUNC_I2C; break;
            case PinFunction::UART: f = GPIO_FUNC_UART; break;
            case PinFunction::PWM:  f = GPIO_FUNC_PWM; break;
            default:                f = GPIO_FUNC_SIO; break;
        }
        gpio_set_function(pin, f);
    }

    void write(uint32_t pin, bool value) override {
        gpio_put(pin, value ? 1 : 0);
    }

    bool read(uint32_t pin) override {
        return gpio_get(pin);
    }

    void toggle(uint32_t pin) override {
        // Alterna o estado do pino sem precisar ler
        gpio_xor_mask(1u << pin);
    }
};
