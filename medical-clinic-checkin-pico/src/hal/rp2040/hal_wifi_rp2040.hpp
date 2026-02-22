#pragma once

#include "hal/interfaces/hal_wifi_interface.hpp"
#include "pico/cyw43_arch.h"

class HAL_WiFi_RP2040 : public HAL_WiFi_Interface {
public:
    HAL_WiFi_RP2040();
    ~HAL_WiFi_RP2040() override;

    bool init(const char* country_code = "BR") override;
    void deinit() override;

    bool connect(const char* ssid,
                 const char* password = nullptr,
                 WiFiSecurityMode sec = WiFiSecurityMode::WPA2,
                 uint32_t timeout_ms = 10000) override;

    void disconnect() override;

    WiFiStatus getStatus() override;

    bool getIPAddress(char* ip_str, size_t size) override;
    bool getMACAddress(uint8_t* mac) override;
    int8_t getRSSI() override;
    bool getSSID(char* ssid, size_t size) override;

    // Stubs
    int32_t scanNetworks(WiFiNetworkInfo*, uint8_t, uint32_t) override { return -1; }
    bool setPowerSaveMode(bool) override { return false; }
    bool setHostname(const char*) override { return false; }
    void setLED(bool) override {}
    bool startAP(const char*, const char*, uint8_t) override { return false; }
    bool stopAP() override { return false; }

    bool ping(const char* host, int attempts, uint32_t timeout_ms) override;
    bool syncTimeNTP(const char* server, uint32_t timeout_ms) override;

private:
    bool       initialized_; // Com underline
    WiFiStatus status_;      // Com underline
};