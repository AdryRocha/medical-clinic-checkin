#pragma once

#include "hal/interfaces/hal_wifi_interface.hpp"

class HAL_WIFI_RP2350 : public HAL_WiFi_Interface {
public:
    HAL_WIFI_RP2350();

    // --- Overrides Obrigatórios (Devem bater 100% com a Interface) ---
    
    // Init agora recebe country_code (padrão "BR" já está na interface)
    bool init(const char* country_code) override;
    void deinit() override;

    // Connect com parâmetros extras de segurança e timeout
    bool connect(const char* ssid, 
                 const char* pwd, 
                 WiFiSecurityMode sec, 
                 uint32_t timeout_ms) override;

    // Disconnect agora é void
    void disconnect() override;

    // GetStatus obrigatório
    WiFiStatus getStatus() override;

    // --- Métodos Opcionais mas úteis ---
    bool isConnected() override;
    bool getIPAddress(char* ip_str, size_t size) override;
    bool getMACAddress(uint8_t* mac) override;
};