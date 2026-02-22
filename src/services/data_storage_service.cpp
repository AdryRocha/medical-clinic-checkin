#include "services/data_storage_service.hpp"
#include "hw_config.h"
#include "pico/stdlib.h"
#include "hardware/rtc.h"
#include <cJSON.h>
#include <cstdio>
#include <cstring>

DataStorageService& DataStorageService::getInstance() {
    static DataStorageService instance;
    return instance;
}

DataStorageService::DataStorageService() 
    : mounted_(false) {
}

DataStorageService::~DataStorageService() {
}

// ============================================================================
// Pré-init do hardware do SD com Pull-up no MISO para evitar ruídos
// ============================================================================
static void sd_hw_preinit() {
    // Pinos exatos mapeados no seu hw_config.c
    const uint miso = 12;
    const uint cs   = 13;

    // O SEGREDO 1: Pull-up no pino MISO para estabilizar a leitura na protoboard
    gpio_pull_up(miso); 
    
    // Configura o Chip Select inicialmente alto e com pull-up
    gpio_init(cs);
    gpio_set_dir(cs, GPIO_OUT);
    gpio_put(cs, 1);
    gpio_pull_up(cs); 

    sleep_ms(50); // Dá um tempo para o hardware estabilizar antes do FatFs
}

bool DataStorageService::init() {
    if (mounted_) {
        return true;
    }
    
    sd_card_t *pSD = sd_get_by_num(0);
    if (!pSD) {
        printf("[DataStorage] ERROR: No SD card configured\n");
        return false;
    }

    // 1. Aplica a proteção contra ruídos no hardware
    sd_hw_preinit();
    
    // 2. O SEGREDO 2: Montagem Adiada (Mudamos o '1' para '0' no final)
    FRESULT fr = f_mount(&fatfs_, pSD->pcName, 0);
    if (FR_OK != fr) {
        printf("[DataStorage] ERROR: Failed to mount SD card: %s (%d)\n", 
               FRESULT_str(fr), fr);
        return false;
    }
    
    mounted_ = true;
    printf("[DataStorage] SD card mounted (Delayed)\n");
    
    fr = f_mkdir("appointments");
    if (FR_OK != fr && FR_EXIST != fr) {
        printf("[DataStorage] ERROR: Failed to create appointments dir: %s\n", 
               FRESULT_str(fr));
        return false;
    }
    
    fr = f_mkdir("fingerprints");
    if (FR_OK != fr && FR_EXIST != fr) {
        printf("[DataStorage] ERROR: Failed to create fingerprints dir: %s\n", 
               FRESULT_str(fr));
        return false;
    }
    
    return true;
}

std::string DataStorageService::getFilePath(const std::string& date) {
    return "appointments/" + date + ".json";
}

bool DataStorageService::saveAppointments(const std::string& date, 
                                         const std::string& json_data) {
    if (!mounted_) {
        printf("[DataStorage] ERROR: SD card not mounted\n");
        return false;
    }
    
    std::string filepath = getFilePath(date);
    
    FIL fil;
    FRESULT fr = f_open(&fil, filepath.c_str(), FA_WRITE | FA_CREATE_ALWAYS);
    
    if (FR_OK != fr) {
        printf("[DataStorage] ERROR: Failed to open file %s: %s\n", 
               filepath.c_str(), FRESULT_str(fr));
        return false;
    }
    
    UINT bytes_written;
    fr = f_write(&fil, json_data.c_str(), json_data.length(), &bytes_written);
    
    f_close(&fil);
    
    if (FR_OK != fr || bytes_written != json_data.length()) {
        printf("[DataStorage] ERROR: Failed to write file %s\n", filepath.c_str());
        return false;
    }
    
    return true;
}

bool DataStorageService::hasAppointments(const std::string& date) {
    if (!mounted_) {
        return false;
    }
    
    std::string filepath = getFilePath(date);
    
    FILINFO fno;
    FRESULT fr = f_stat(filepath.c_str(), &fno);
    
    return (FR_OK == fr);
}

bool DataStorageService::readAppointments(const std::string& date, 
                                         std::string& json_data) {
    if (!mounted_) {
        printf("[DataStorage] ERROR: SD card not mounted\n");
        return false;
    }
    
    std::string filepath = getFilePath(date);
    
    FIL fil;
    FRESULT fr = f_open(&fil, filepath.c_str(), FA_READ);
    
    if (FR_OK != fr) {
        printf("[DataStorage] ERROR: Failed to open file %s: %s\n", 
               filepath.c_str(), FRESULT_str(fr));
        return false;
    }
    
    FSIZE_t size = f_size(&fil);
    
    char* buffer = new char[size + 1];
    if (!buffer) {
        f_close(&fil);
        printf("[DataStorage] ERROR: Failed to allocate memory\n");
        return false;
    }
    
    UINT bytes_read;
    fr = f_read(&fil, buffer, size, &bytes_read);
    
    f_close(&fil);
    
    if (FR_OK != fr || bytes_read != size) {
        delete[] buffer;
        printf("[DataStorage] ERROR: Failed to read file %s\n", filepath.c_str());
        return false;
    }
    
    buffer[bytes_read] = '\0';
    json_data = std::string(buffer);
    delete[] buffer;
    
    return true;
}

std::string DataStorageService::getTodayDate() {
    datetime_t dt;
    if (!rtc_get_datetime(&dt)) {
        return "";
    }
    
    char date_str[16];
    snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d", 
             dt.year, dt.month, dt.day);
    
    return std::string(date_str);
}

std::string DataStorageService::getTomorrowDate() {
    datetime_t dt;
    if (!rtc_get_datetime(&dt)) {
        return "";
    }
    
    int day = dt.day + 1;
    int month = dt.month;
    int year = dt.year;
    
    // Days in each month
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // Adjust February for leap years
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        days_in_month[1] = 29;
    }
    
    if (day > days_in_month[month - 1]) {
        day = 1;
        month++;
        if (month > 12) {
            month = 1;
            year++;
        }
    }
    
    char date_str[16];
    snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d", year, month, day);
    
    return std::string(date_str);
}

bool DataStorageService::isReady() {
    return mounted_;
}

bool DataStorageService::markAppointmentCompleted(const std::string& date, int appointment_id) {
    if (!mounted_) {
        printf("[DataStorage] ERROR: SD card not mounted\n");
        return false;
    }
    
    std::string json_data;
    if (!readAppointments(date, json_data)) {
        return false;
    }
    
    cJSON* root = cJSON_Parse(json_data.c_str());
    if (!root || !cJSON_IsArray(root)) {
        if (root) cJSON_Delete(root);
        return false;
    }
    
    bool found = false;
    int array_size = cJSON_GetArraySize(root);
    
    for (int i = 0; i < array_size; i++) {
        cJSON* appointment = cJSON_GetArrayItem(root, i);
        if (!appointment) continue;
        
        cJSON* id_json = cJSON_GetObjectItem(appointment, "id");
        if (!id_json || !cJSON_IsNumber(id_json)) continue;
        
        if (id_json->valueint == appointment_id) {
            cJSON* status_json = cJSON_GetObjectItem(appointment, "status");
            if (status_json) {
                cJSON_SetValuestring(status_json, "realizada");
            } else {
                cJSON_AddStringToObject(appointment, "status", "realizada");
            }
            found = true;
            break;
        }
    }
    
    if (!found) {
        cJSON_Delete(root);
        return false;
    }
    
    char* json_string = cJSON_Print(root);
    cJSON_Delete(root);
    
    if (!json_string) {
        return false;
    }
    
    std::string updated_json(json_string);
    cJSON_free(json_string);
    
    return saveAppointments(date, updated_json);
}

bool DataStorageService::isAppointmentCompleted(const std::string& date, int appointment_id) {
    if (!mounted_) {
        return false;
    }
    
    std::string json_data;
    if (!readAppointments(date, json_data)) {
        return false;
    }
    
    cJSON* root = cJSON_Parse(json_data.c_str());
    if (!root || !cJSON_IsArray(root)) {
        if (root) cJSON_Delete(root);
        return false;
    }
    
    bool is_completed = false;
    int array_size = cJSON_GetArraySize(root);
    
    for (int i = 0; i < array_size; i++) {
        cJSON* appointment = cJSON_GetArrayItem(root, i);
        if (!appointment) continue;
        
        cJSON* id_json = cJSON_GetObjectItem(appointment, "id");
        if (!id_json || !cJSON_IsNumber(id_json)) continue;
        
        if (id_json->valueint == appointment_id) {
            cJSON* status_json = cJSON_GetObjectItem(appointment, "status");
            if (status_json && cJSON_IsString(status_json)) {
                is_completed = (strcmp(status_json->valuestring, "realizada") == 0);
            }
            break;
        }
    }
    
    cJSON_Delete(root);
    return is_completed;
}

bool DataStorageService::saveFingerprintTemplate(int patient_id, 
                                                 const std::vector<uint8_t>& template_data) {
    if (!mounted_) {
        printf("[DataStorage] ERROR: SD card not mounted\n");
        return false;
    }
    
    char filepath[64];
    snprintf(filepath, sizeof(filepath), "fingerprints/%d.dat", patient_id);
    
    FIL fil;
    FRESULT fr = f_open(&fil, filepath, FA_WRITE | FA_CREATE_ALWAYS);
    
    if (FR_OK != fr) {
        printf("[DataStorage] ERROR: Failed to open fingerprint file %s: %s\n",
               filepath, FRESULT_str(fr));
        return false;
    }
    
    UINT bytes_written;
    fr = f_write(&fil, template_data.data(), template_data.size(), &bytes_written);
    
    f_close(&fil);
    
    if (FR_OK != fr || bytes_written != template_data.size()) {
        printf("[DataStorage] ERROR: Failed to write fingerprint file %s\n", filepath);
        return false;
    }
    
    return true;
}

bool DataStorageService::loadFingerprintTemplate(int patient_id,
                                                 std::vector<uint8_t>& template_data) {
    if (!mounted_) {
        printf("[DataStorage] ERROR: SD card not mounted\n");
        return false;
    }
    
    char filepath[64];
    snprintf(filepath, sizeof(filepath), "fingerprints/%d.dat", patient_id);
    
    FIL fil;
    FRESULT fr = f_open(&fil, filepath, FA_READ);
    
    if (FR_OK != fr) {
        printf("[DataStorage] ERROR: Failed to open fingerprint file %s: %s\n",
               filepath, FRESULT_str(fr));
        return false;
    }
    
    FSIZE_t size = f_size(&fil);
    template_data.resize(size);
    
    UINT bytes_read;
    fr = f_read(&fil, template_data.data(), size, &bytes_read);
    
    f_close(&fil);
    
    if (FR_OK != fr || bytes_read != size) {
        printf("[DataStorage] ERROR: Failed to read fingerprint file %s\n", filepath);
        return false;
    }
    
    return true;
}

bool DataStorageService::hasFingerprintTemplate(int patient_id) {
    if (!mounted_) {
        return false;
    }
    
    char filepath[64];
    snprintf(filepath, sizeof(filepath), "fingerprints/%d.dat", patient_id);
    
    FILINFO fno;
    FRESULT fr = f_stat(filepath, &fno);
    
    return (FR_OK == fr);
}

bool DataStorageService::deleteFingerprintTemplate(int patient_id) {
    if (!mounted_) {
        printf("[DataStorage] ERROR: SD card not mounted\n");
        return false;
    }
    
    char filepath[64];
    snprintf(filepath, sizeof(filepath), "fingerprints/%d.dat", patient_id);
    
    FRESULT fr = f_unlink(filepath);
    
    if (FR_OK != fr && FR_NO_FILE != fr) {
        printf("[DataStorage] ERROR: Failed to delete fingerprint %s: %s\n",
               filepath, FRESULT_str(fr));
        return false;
    }
    
    return true;
}

bool DataStorageService::clearAllFingerprints() {
    if (!mounted_) {
        printf("[DataStorage] ERROR: SD card not mounted\n");
        return false;
    }
    
    DIR dir;
    FRESULT fr = f_opendir(&dir, "fingerprints");
    
    if (FR_OK != fr) {
        printf("[DataStorage] ERROR: Failed to open fingerprints directory\n");
        return false;
    }
    
    FILINFO fno;
    
    while (true) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) break;
        
        if (!(fno.fattrib & AM_DIR)) {
            char filepath[128];
            snprintf(filepath, sizeof(filepath), "fingerprints/%s", fno.fname);
            f_unlink(filepath);
        }
    }
    
    f_closedir(&dir);
    return true;
}