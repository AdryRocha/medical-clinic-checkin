#pragma once
#ifndef HAL_WIFI_INTERFACE_HPP
#define HAL_WIFI_INTERFACE_HPP

#include <cstddef>
#include <cstdint>

enum class WiFiStatus : uint8_t {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
};

enum class WiFiSecurityMode : uint8_t {
    OPEN,
    WPA2,
    WPA3
};

struct WiFiNetworkInfo {
    char ssid[33]{};
    int8_t rssi = 0;
    uint8_t channel = 0;
    WiFiSecurityMode security = WiFiSecurityMode::OPEN;
};

class HAL_WiFi_Interface {
public:
    virtual ~HAL_WiFi_Interface() = default;

    // API base (que seu código usa)
    virtual bool init(const char* country_code = "BR") = 0;
    virtual void deinit() = 0;

    virtual bool connect(const char* ssid,
                         const char* pwd,
                         WiFiSecurityMode sec = WiFiSecurityMode::WPA2,
                         uint32_t timeout_ms = 30000) = 0;

    virtual void disconnect() = 0;
    virtual WiFiStatus getStatus() = 0;

    // Conveniência
    virtual bool isConnected() { return getStatus() == WiFiStatus::CONNECTED; }

    // Extras (não obrigatórios)
    virtual bool getIPAddress(char*, size_t) { return false; }
    virtual bool getMACAddress(uint8_t*) { return false; }
    virtual int8_t getRSSI() { return 0; }
    virtual bool getSSID(char*, size_t) { return false; }
    virtual int32_t scanNetworks(WiFiNetworkInfo*, uint8_t, uint32_t) { return -1; }
    virtual bool ping(const char*, int, uint32_t) { return false; }
    virtual bool syncTimeNTP(const char*, uint32_t) { return false; }
};

#endif
