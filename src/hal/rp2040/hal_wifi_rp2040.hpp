#ifndef HAL_WIFI_RP2040_HPP
#define HAL_WIFI_RP2040_HPP

#include "hal/interfaces/hal_wifi_interface.hpp"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"

/**
 * @brief RP2040 Pico W-specific WiFi HAL implementation
 * 
 * This implementation uses the CYW43439 wireless chip found on the
 * Raspberry Pi Pico W board, along with lwIP for networking stack.
 * 
 * @note Requires PICO_BOARD=pico_w in CMakeLists.txt
 * @note Requires pico_cyw43_arch_lwip_sys_freertos library (FreeRTOS integration)
 */
class HAL_WiFi_RP2040 : public HAL_WiFi_Interface {
private:
    bool initialized_;
    bool ap_mode_;
    char current_ssid_[33];
    char hostname_[64];
    
    /**
     * @brief Convert CYW43 auth mode to our security enum
     */
    WiFiSecurityMode convertAuthMode(uint32_t auth_mode);
    
    /**
     * @brief Convert our security enum to CYW43 auth mode
     */
    uint32_t convertToAuthMode(WiFiSecurityMode security);

public:
    /**
     * @brief Construct an RP2040 WiFi HAL instance
     */
    HAL_WiFi_RP2040();
    
    /**
     * @brief Destructor - ensures WiFi is properly deinitialized
     */
    ~HAL_WiFi_RP2040() override;

    // HAL_WiFi_Interface implementation
    bool init(const char* country_code = "BR") override;
    void deinit() override;
    
    bool connect(const char* ssid, const char* password = nullptr,
                WiFiSecurityMode security = WiFiSecurityMode::WPA2_AES_PSK,
                uint32_t timeout_ms = 30000) override;
    
    bool isConnected() override;
    WiFiStatus getStatus() override;
    
    bool getIPAddress(char* ip_str, size_t buffer_size) override;
    bool getMACAddress(uint8_t* mac) override;
    int8_t getRSSI() override;
    bool getSSID(char* ssid, size_t buffer_size) override;
    
    int32_t scanNetworks(WiFiNetworkInfo* networks, uint8_t max_networks,
                        uint32_t timeout_ms = 10000) override;
    
    bool setPowerSaveMode(bool enable) override;
    bool setHostname(const char* hostname) override;
    
    bool testDNSConnectivity(const char* host, 
                            uint32_t timeout_ms = 5000) override;
    
    void setLED(bool state) override;
    
    bool startAP(const char* ssid, const char* password = nullptr,
                uint8_t channel = 1) override;
    
    bool stopAP() override;
};

#endif // HAL_WIFI_RP2040_HPP
