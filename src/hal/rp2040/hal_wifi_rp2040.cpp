#include "hal/rp2040/hal_wifi_rp2040.hpp"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"
#include "lwip/apps/lwiperf.h"
#include "lwip/icmp.h"
#include "lwip/inet_chksum.h"
#include "lwip/raw.h"
#include "FreeRTOS.h"
#include "task.h"
#include <cstring>
#include <stdio.h>

HAL_WiFi_RP2040::HAL_WiFi_RP2040() 
    : initialized_(false), ap_mode_(false) {
    current_ssid_[0] = '\0';
    strcpy(hostname_, "pico-w");
}

HAL_WiFi_RP2040::~HAL_WiFi_RP2040() {
    if (initialized_) {
        deinit();
    }
}

bool HAL_WiFi_RP2040::init(const char* country_code) {
    if (initialized_) {
        return true; // Already initialized
    }

    // Initialize CYW43 with lwIP stack and country code (FreeRTOS sys integration)
    int result = cyw43_arch_init_with_country(
        CYW43_COUNTRY(country_code[0], country_code[1], 0));
    
    if (result != 0) {
        return false;
    }

    // Enable station mode by default
    cyw43_arch_enable_sta_mode();
    
    initialized_ = true;
    
    return true;
}

void HAL_WiFi_RP2040::deinit() {
    if (!initialized_) {
        return;
    }

    if (ap_mode_) {
        stopAP();
    }

    cyw43_arch_deinit();
    initialized_ = false;
    current_ssid_[0] = '\0';
}

bool HAL_WiFi_RP2040::connect(const char* ssid, const char* password,
                              WiFiSecurityMode security, uint32_t timeout_ms) {
    if (!initialized_) {
        return false;
    }

    if (ap_mode_) {
        return false; // Cannot connect in AP mode
    }

    uint32_t auth_mode = convertToAuthMode(security);
    
    // Attempt connection with timeout
    int result = cyw43_arch_wifi_connect_timeout_ms(
        ssid, 
        password ? password : "", 
        auth_mode,
        timeout_ms
    );

    if (result != 0) {
        return false;
    }

    // Store SSID for later retrieval
    strncpy(current_ssid_, ssid, sizeof(current_ssid_) - 1);
    current_ssid_[sizeof(current_ssid_) - 1] = '\0';

    return true;
}

bool HAL_WiFi_RP2040::isConnected() {
    if (!initialized_) {
        return false;
    }

    int link_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    return (link_status == CYW43_LINK_UP);
}

WiFiStatus HAL_WiFi_RP2040::getStatus() {
    if (!initialized_) {
        return WiFiStatus::DISCONNECTED;
    }

    int link_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    
    switch (link_status) {
        case CYW43_LINK_DOWN:
            return WiFiStatus::DISCONNECTED;
        case CYW43_LINK_JOIN:
            return WiFiStatus::CONNECTING;
        case CYW43_LINK_UP:
            return WiFiStatus::CONNECTED;
        case CYW43_LINK_FAIL:
            return WiFiStatus::FAILED;
        case CYW43_LINK_NONET:
            return WiFiStatus::NO_SSID_AVAILABLE;
        case CYW43_LINK_BADAUTH:
            return WiFiStatus::FAILED;
        default:
            return WiFiStatus::DISCONNECTED;
    }
}

bool HAL_WiFi_RP2040::getIPAddress(char* ip_str, size_t buffer_size) {
    if (!initialized_ || !isConnected() || buffer_size < 16) {
        return false;
    }

    struct netif *netif = netif_default;
    if (netif == NULL) {
        return false;
    }

    const ip4_addr_t *addr = netif_ip4_addr(netif);
    snprintf(ip_str, buffer_size, "%s", ip4addr_ntoa(addr));
    
    return true;
}

bool HAL_WiFi_RP2040::getMACAddress(uint8_t* mac) {
    if (!initialized_ || mac == nullptr) {
        return false;
    }

    // Get MAC address from CYW43
    cyw43_hal_get_mac(CYW43_ITF_STA, mac);
    
    return true;
}

int8_t HAL_WiFi_RP2040::getRSSI() {
    if (!initialized_ || !isConnected()) {
        return 0;
    }

    int32_t rssi;
    if (cyw43_wifi_get_rssi(&cyw43_state, &rssi) == 0) {
        return (int8_t)rssi;
    }

    return 0;
}

bool HAL_WiFi_RP2040::getSSID(char* ssid, size_t buffer_size) {
    if (!initialized_ || !isConnected() || buffer_size < 33) {
        return false;
    }

    strncpy(ssid, current_ssid_, buffer_size - 1);
    ssid[buffer_size - 1] = '\0';
    
    return true;
}

// Scan callback data structure
struct scan_result_data {
    WiFiNetworkInfo* networks;
    uint8_t max_networks;
    uint8_t count;
    bool scan_complete;
};

int32_t HAL_WiFi_RP2040::scanNetworks(WiFiNetworkInfo* networks, 
                                      uint8_t max_networks,
                                      uint32_t timeout_ms) {
    if (!initialized_ || networks == nullptr || max_networks == 0) {
        return -1;
    }

    // Setup scan result data structure
    scan_result_data scan_data = {
        .networks = networks,
        .max_networks = max_networks,
        .count = 0,
        .scan_complete = false
    };

    cyw43_wifi_scan_options_t scan_options = {0};
    int result = cyw43_wifi_scan(&cyw43_state, &scan_options, &scan_data, 
                                 [](void *env, const cyw43_ev_scan_result_t *result) -> int {
        auto* data = static_cast<scan_result_data*>(env);
        
        if (!result) {
            // Scan complete
            data->scan_complete = true;
            return 0;
        }

        // Check if we have space for more results
        if (data->count >= data->max_networks) {
            return 0; // No more space, but continue scanning
        }

        WiFiNetworkInfo* network = &data->networks[data->count];
        
        // Copy SSID
        size_t ssid_len = result->ssid_len;
        if (ssid_len > 32) ssid_len = 32;
        memcpy(network->ssid, result->ssid, ssid_len);
        network->ssid[ssid_len] = '\0';
        
        // Copy BSSID (MAC address)
        memcpy(network->bssid, result->bssid, 6);
        
        // Store RSSI and channel
        network->rssi = result->rssi;
        network->channel = result->channel;
        
        // Convert auth mode to our security enum
        switch (result->auth_mode) {
            case CYW43_AUTH_OPEN:
                network->security = WiFiSecurityMode::OPEN;
                break;
            case CYW43_AUTH_WPA2_AES_PSK:
            case CYW43_AUTH_WPA2_MIXED_PSK:
                network->security = WiFiSecurityMode::WPA2_AES_PSK;
                break;
            default:
                network->security = WiFiSecurityMode::WPA2_AES_PSK;
                break;
        }
        
        data->count++;
        
        return 0; // Continue scanning
    });

    if (result != 0) {
        return -1;
    }

    // Wait for scan to complete or timeout (FreeRTOS handles polling)
    uint32_t start = to_ms_since_boot(get_absolute_time());
    while (!scan_data.scan_complete && 
           (to_ms_since_boot(get_absolute_time()) - start) < timeout_ms) {
        vTaskDelay(pdMS_TO_TICKS(100));  // Yield to FreeRTOS
    }
    
    return scan_data.count;
}

bool HAL_WiFi_RP2040::setPowerSaveMode(bool enable) {
    if (!initialized_) {
        return false;
    }

    uint32_t pm_mode = enable ? CYW43_DEFAULT_PM : CYW43_NO_POWERSAVE_MODE;
    
    int result = cyw43_wifi_pm(&cyw43_state, pm_mode);
    
    return (result == 0);
}

bool HAL_WiFi_RP2040::setHostname(const char* hostname) {
    if (!initialized_ || hostname == nullptr) {
        return false;
    }

    strncpy(hostname_, hostname, sizeof(hostname_) - 1);
    hostname_[sizeof(hostname_) - 1] = '\0';
    
    // Set hostname in lwIP stack
    netif_set_hostname(netif_default, hostname_);
    
    return true;
}

bool HAL_WiFi_RP2040::testDNSConnectivity(const char* host, 
                                          uint32_t timeout_ms) {
    if (!initialized_ || !isConnected() || host == nullptr) {
        return false;
    }

    ip_addr_t addr;
    err_t err = dns_gethostbyname(host, &addr, NULL, NULL);
    
    if (err == ERR_OK) {
        // DNS resolved immediately (cached or numeric IP)
        return true;
    } else if (err == ERR_INPROGRESS) {
        // DNS lookup in progress - wait for result
        uint32_t start = to_ms_since_boot(get_absolute_time());
        while ((to_ms_since_boot(get_absolute_time()) - start) < timeout_ms) {
            // Check if DNS resolved
            err = dns_gethostbyname(host, &addr, NULL, NULL);
            if (err == ERR_OK) {
                return true;
            }
            
            vTaskDelay(pdMS_TO_TICKS(100));  // Check every 100ms
        }
        
        return false; // Timeout
    } else {
        // DNS lookup failed immediately
        return false;
    }
}

void HAL_WiFi_RP2040::setLED(bool state) {
    if (!initialized_) {
        return;
    }
    
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state ? 1 : 0);
}

bool HAL_WiFi_RP2040::startAP(const char* ssid, const char* password, 
                              uint8_t channel) {
    if (!initialized_) {
        return false;
    }

    uint32_t auth_mode = (password && strlen(password) >= 8) 
                         ? CYW43_AUTH_WPA2_AES_PSK 
                         : CYW43_AUTH_OPEN;

    cyw43_arch_enable_ap_mode(ssid, password ? password : "", auth_mode);
    
    ap_mode_ = true;
    strncpy(current_ssid_, ssid, sizeof(current_ssid_) - 1);
    current_ssid_[sizeof(current_ssid_) - 1] = '\0';
    
    return true;
}

bool HAL_WiFi_RP2040::stopAP() {
    if (!initialized_ || !ap_mode_) {
        return false;
    }

    cyw43_arch_disable_ap_mode();
    cyw43_arch_enable_sta_mode();
    
    ap_mode_ = false;
    current_ssid_[0] = '\0';
    
    return true;
}


WiFiSecurityMode HAL_WiFi_RP2040::convertAuthMode(uint32_t auth_mode) {
    switch (auth_mode) {
        case CYW43_AUTH_OPEN:
            return WiFiSecurityMode::OPEN;
        case CYW43_AUTH_WPA2_AES_PSK:
        case CYW43_AUTH_WPA2_MIXED_PSK:
            return WiFiSecurityMode::WPA2_AES_PSK;
        default:
            return WiFiSecurityMode::WPA2_AES_PSK;
    }
}

uint32_t HAL_WiFi_RP2040::convertToAuthMode(WiFiSecurityMode security) {
    switch (security) {
        case WiFiSecurityMode::OPEN:
            return CYW43_AUTH_OPEN;
        case WiFiSecurityMode::WPA_TKIP_PSK:
        case WiFiSecurityMode::WPA2_AES_PSK:
        case WiFiSecurityMode::WPA2_MIXED_PSK:
            return CYW43_AUTH_WPA2_AES_PSK;
        default:
            return CYW43_AUTH_WPA2_AES_PSK;
    }
}
