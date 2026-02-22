#pragma once

#include "ff.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <cstdint>
#include <cstddef>
#include <string>

// Estrutura para credenciais WiFi
struct WifiCredentials {
    char ssid[33];
    char password[64];
    bool valid;
};

class DataStorageService {
public:
    static bool init();

    static bool save_wifi(const char* ssid, const char* password);
    static bool load_wifi(WifiCredentials* out_creds);

    static bool append_log(const char* filename, const char* content);

    static bool write_file(const char* filename,
                           const uint8_t* data,
                           size_t size);

    static size_t read_file(const char* filename,
                            uint8_t* out_buffer,
                            size_t max_size);

    static bool file_exists(const char* filename);
    static bool is_mounted();

private:
    static bool mounted_;
    static SemaphoreHandle_t mutex_;

    static bool take_fs();
    static void give_fs();
};
