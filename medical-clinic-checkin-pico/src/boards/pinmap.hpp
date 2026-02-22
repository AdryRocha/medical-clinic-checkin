#pragma once
#include <cstdint>

#define DISPLAY_PIN_BLK 22  // Or your actual pin

namespace BoardPico2W {

struct PinMap {
    // DISPLAY ST7796 (SPI0)
    static constexpr unsigned int DISP_SPI_SCK  = 18;
    static constexpr unsigned int DISP_SPI_MOSI = 19;
    static constexpr unsigned int DISP_SPI_MISO = 16; 
    static constexpr unsigned int DISP_CS       = 17;
    static constexpr unsigned int DISP_DC       = 20; 
    static constexpr unsigned int DISP_RST      = 21; 
    static constexpr unsigned int DISP_BL       = 22;

    // TOUCH FT6336U (I2C1)
    static constexpr unsigned int TOUCH_PIN_SDA = 6;
    static constexpr unsigned int TOUCH_PIN_SCL = 7;
    static constexpr unsigned int TOUCH_PIN_RST = 8;
    static constexpr unsigned int TOUCH_PIN_IRQ = 9;

    // SD CARD (SPI1)
    static constexpr unsigned int SD_SPI_SCK  = 14;
    static constexpr unsigned int SD_SPI_MOSI = 15;
    static constexpr unsigned int SD_SPI_MISO = 12;
    static constexpr unsigned int SD_CS       = 13;

    // QR SCANNER (UART0)
    static constexpr unsigned int UART0_TX = 0;
    static constexpr unsigned int UART0_RX = 1;
    static constexpr unsigned int QR_TX = 0; // Alias
    static constexpr unsigned int QR_RX = 1; // Alias

    // FINGERPRINT (UART1)
    static constexpr unsigned int UART1_TX = 4;
    static constexpr unsigned int UART1_RX = 5;
    static constexpr unsigned int FP_TX = 4; // Alias
    static constexpr unsigned int FP_RX = 5; // Alias
};

} // namespace BoardPico2W