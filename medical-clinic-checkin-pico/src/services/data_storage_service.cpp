#include "services/data_storage_service.hpp"
#include "services/logger_service.hpp"

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

#include <cstring>
#include <cstdio>

// FatFS / SD
#include "hw_config.h"
#include "ff.h"

// ============================================================================
// Static members
// ============================================================================
bool DataStorageService::mounted_ = false;
SemaphoreHandle_t DataStorageService::mutex_ = nullptr;

// ============================================================================
// Mutex helpers
// ============================================================================
bool DataStorageService::take_fs() {
    if (!mutex_) return false;
    return xSemaphoreTake(mutex_, pdMS_TO_TICKS(2000)) == pdTRUE;
}

void DataStorageService::give_fs() {
    if (mutex_) xSemaphoreGive(mutex_);
}

// ============================================================================
// Pré-init do hardware do SD com Pull-up no MISO
// ============================================================================
static void sd_hw_preinit(sd_card_t* sd)
{
    if (!sd || !sd->spi || !sd->spi->hw_inst) return;

    spi_inst_t* spi = sd->spi->hw_inst;

    const uint miso = sd->spi->miso_gpio;
    const uint mosi = sd->spi->mosi_gpio;
    const uint sck  = sd->spi->sck_gpio;
    const uint cs   = sd->ss_gpio;

    uint32_t baud = sd->spi->baud_rate;
    if (baud < 200 * 1000) baud = 400 * 1000; 

    spi_init(spi, baud);
    spi_set_format(spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(miso, GPIO_FUNC_SPI);
    gpio_set_function(mosi, GPIO_FUNC_SPI);
    gpio_set_function(sck,  GPIO_FUNC_SPI);

    gpio_pull_up(miso); // Garante estabilidade na leitura do SD
    // ------------------------------

    gpio_init(cs);
    gpio_set_dir(cs, GPIO_OUT);
    gpio_put(cs, 1);
    gpio_pull_up(cs); 

    sleep_ms(50);
}

// ============================================================================
// Init (Simplificado para evitar travamentos no Boot)
// ============================================================================
bool DataStorageService::init()
{
    LOGGER_INFO("[STORAGE] Entrando em init()...");
    if (!mutex_) {
        mutex_ = xSemaphoreCreateMutex();
    }

    if (!take_fs()) return false;

    sd_card_t* sd = sd_get_by_num(0);
    if (!sd) {
        LOGGER_ERROR("[STORAGE] Hardware SD não encontrado no config!");
        give_fs();
        return false;
    }

    LOGGER_INFO("[STORAGE] Pré-inicializando hardware SPI...");
    sd_hw_preinit(sd);

    LOGGER_INFO("[STORAGE] Tentando montar (modo adiado)...");
    // MUDANÇA: Usamos 0 em vez de 1 para não travar o boot aqui
    FRESULT fr = f_mount(&sd->fatfs, sd->pcName, 0); 
    
    if (fr != FR_OK) {
        LOGGER_ERROR("[STORAGE] Erro lógico no f_mount: %d", fr);
        mounted_ = false;
        give_fs();
        return false;
    }

    mounted_ = true;
    LOGGER_INFO("[STORAGE] SD configurado. Montagem real ocorrerá no primeiro acesso.");
    give_fs();
    return true;
}

// ============================================================================
// WiFi config
// ============================================================================
bool DataStorageService::save_wifi(const char* ssid, const char* password)
{
    if (!mounted_ || !ssid || !password || !take_fs())
        return false;

    FIL file;
    char buffer[128];
    int len = snprintf(buffer, sizeof(buffer), "%s\n%s", ssid, password);

    FRESULT fr = f_open(&file, "wifi_config.txt", FA_WRITE | FA_CREATE_ALWAYS);
    if (fr == FR_OK) {
        UINT written = 0;
        f_write(&file, buffer, len, &written);
        f_close(&file);
        LOGGER_INFO("[STORAGE] WiFi salvo");
    } else {
        LOGGER_ERROR("[STORAGE] Erro ao salvar WiFi: %d", fr);
    }

    give_fs();
    return (fr == FR_OK);
}

bool DataStorageService::load_wifi(WifiCredentials* out_creds)
{
    if (!mounted_ || !out_creds || !take_fs())
        return false;

    out_creds->valid = false;

    FIL file;
    char buffer[128] = {0};
    UINT read_len = 0;

    FRESULT fr = f_open(&file, "wifi_config.txt", FA_READ);
    if (fr == FR_OK) {
        f_read(&file, buffer, sizeof(buffer) - 1, &read_len);
        f_close(&file);
    }

    give_fs();

    if (fr != FR_OK || read_len == 0)
        return false;

    char* line1 = strtok(buffer, "\n\r");
    char* line2 = strtok(nullptr, "\n\r");

    if (!line1 || strlen(line1) == 0)
        return false;

    strncpy(out_creds->ssid, line1, sizeof(out_creds->ssid) - 1);
    out_creds->ssid[sizeof(out_creds->ssid) - 1] = '\0';

    if (line2) {
        strncpy(out_creds->password, line2, sizeof(out_creds->password) - 1);
        out_creds->password[sizeof(out_creds->password) - 1] = '\0';
    } else {
        out_creds->password[0] = '\0';
    }

    out_creds->valid = true;
    return true;
}

// ============================================================================
// Binary files
// ============================================================================
bool DataStorageService::write_file(const char* filename,
                                   const uint8_t* data,
                                   size_t size)
{
    if (!mounted_ || !filename || !data || size == 0 || !take_fs())
        return false;

    FIL file;
    UINT written = 0;

    FRESULT fr = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr == FR_OK) {
        f_write(&file, data, size, &written);
        f_close(&file);
    } else {
        LOGGER_ERROR("[STORAGE] Erro ao escrever %s: %d", filename, fr);
    }

    give_fs();
    return (fr == FR_OK && written == size);
}

size_t DataStorageService::read_file(const char* filename,
                                    uint8_t* out_buffer,
                                    size_t max_size)
{
    if (!mounted_ || !filename || !out_buffer || max_size == 0 || !take_fs())
        return 0;

    FIL file;
    UINT read_bytes = 0;

    FRESULT fr = f_open(&file, filename, FA_READ);
    if (fr == FR_OK) {
        f_read(&file, out_buffer, max_size, &read_bytes);
        f_close(&file);
    } else {
        LOGGER_WARN("[STORAGE] Arquivo %s não encontrado", filename);
    }

    give_fs();
    return (fr == FR_OK) ? read_bytes : 0;
}
