#pragma once

#include "hal/interfaces/hal_gpio_interface.hpp"
#include "hal/interfaces/hal_spi_interface.hpp"
#include "hal/interfaces/hal_i2c_interface.hpp"
#include "hal/interfaces/hal_uart_interface.hpp"
#include "hal/interfaces/hal_time_interface.hpp"
#include "hal/interfaces/hal_wifi_interface.hpp"

struct BoardContext {
    HAL_GPIO_Interface* gpio = nullptr;
    
    // SPIs com nomes semânticos
    HAL_SPI_Interface* spi_display = nullptr;
    HAL_SPI_Interface* spi_sd = nullptr;
    
    HAL_I2C_Interface* i2c = nullptr;
    
    HAL_UART_Interface* uart_qr = nullptr;
    
    HAL_UART_Interface* uart_fingerprint = nullptr;
    RingBuffer* fingerprint_buffer = nullptr;
    
    HAL_Time_Interface* time = nullptr;
    HAL_WiFi_Interface* wifi = nullptr;
};