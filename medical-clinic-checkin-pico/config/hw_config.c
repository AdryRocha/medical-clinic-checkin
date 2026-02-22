/* hw_config.c
 * Hardware Configuration for SD Card Driver
 * 
 * Based on carlk3's no-OS-FatFS library.
 */

#include <assert.h>
#include <string.h>
//
#include "hw_config.h"
//
#include "ff.h"
#include "diskio.h"

/* 

SPI1 Configuration (from hardware diagram):
|       | SPI1  | GPIO  | Description            | 
| ----- | ----  | ----- | ---------------------- |
| MISO  | RX    | 12    | Master In, Slave Out   |
| MOSI  | TX    | 15    | Master Out, Slave In   |
| SCK   | SCK   | 14    | SPI clock              |
| CS    | CSn   | 13    | Chip Select            |

Note: No card detect pin in current design (use_card_detect = false)
*/

// Hardware Configuration of SPI "objects"
static spi_t spis[] = {  // One for each SPI.
    {
        .hw_inst = spi1,  // SPI1 component
        .miso_gpio = 12,  // GP12 - MISO
        .mosi_gpio = 15,  // GP15 - MOSI
        .sck_gpio = 14,   // GP14 - SCK
        
        // Start with conservative speed, can be increased after initialization
        // .baud_rate = 1000 * 1000  // 1 MHz for initialization
        .baud_rate = 1000 * 1000    // 12.5 MHz for normal operation
        // .baud_rate = 25 * 1000 * 1000 // 25 MHz max (if needed)
    }
};

// Hardware Configuration of the SD Card "objects"
static sd_card_t sd_cards[] = {  // One for each SD card
    {
        .pcName = "0:",           // Name used to mount device (FatFs logical drive)
        .spi = &spis[0],          // Pointer to the SPI driving this card
        .ss_gpio = 13,            // GP13 - Chip Select
        .use_card_detect = false, // No card detect pin in current design
        .card_detect_gpio = 0,    // Not used
        .card_detected_true = 0   // Not used
    }
};

/* ********************************************************************** */
/* Public API implementation */

size_t sd_get_num() { 
    return count_of(sd_cards); 
}

sd_card_t *sd_get_by_num(size_t num) {
    if (num < sd_get_num()) {
        return &sd_cards[num];
    } else {
        return NULL;
    }
}

size_t spi_get_num() { 
    return count_of(spis); 
}

spi_t *spi_get_by_num(size_t num) {
    if (num < spi_get_num()) {
        return &spis[num];
    } else {
        return NULL;
    }
}

/* [] END OF FILE */