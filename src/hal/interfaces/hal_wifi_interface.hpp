#ifndef HAL_WIFI_INTERFACE_HPP
#define HAL_WIFI_INTERFACE_HPP

#include <stdint.h>
#include <stddef.h>

/**
 * @brief WiFi Security modes
 */
enum class WiFiSecurityMode {
    OPEN = 0,
    WPA_TKIP_PSK = 1,
    WPA2_AES_PSK = 2,
    WPA2_MIXED_PSK = 3,
    WPA3_SAE = 4
};

/**
 * @brief WiFi connection status
 */
enum class WiFiStatus {
    DISCONNECTED = 0,
    CONNECTING = 1,
    CONNECTED = 2,
    FAILED = 3,
    NO_SSID_AVAILABLE = 4,
    CONNECTION_LOST = 5
};

/**
 * @brief WiFi network information
 */
struct WiFiNetworkInfo {
    char ssid[33];              // SSID (max 32 chars + null terminator)
    uint8_t bssid[6];           // MAC address of AP
    int8_t rssi;                // Signal strength in dBm
    uint8_t channel;            // WiFi channel
    WiFiSecurityMode security;   // Security mode
};

/**
 * @brief Hardware Abstraction Layer for WiFi communication
 */
class HAL_WiFi_Interface {
public:
    virtual ~HAL_WiFi_Interface() = default;

    /**
     * @brief Initialize the WiFi hardware
     * @param country_code Two-letter country code (e.g., "BR", "US")
     * @return true if initialization successful
     */
    virtual bool init(const char* country_code = "BR") = 0;

    /**
     * @brief Deinitialize the WiFi hardware
     */
    virtual void deinit() = 0;

    /**
     * @brief Connect to a WiFi network
     * @param ssid Network SSID
     * @param password Network password (NULL for open networks)
     * @param security Security mode
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return true if connection successful
     */
    virtual bool connect(const char* ssid, const char* password = nullptr,
                        WiFiSecurityMode security = WiFiSecurityMode::WPA2_AES_PSK,
                        uint32_t timeout_ms = 30000) = 0;

    /**
     * @brief Check if connected to a network
     * @return true if connected
     */
    virtual bool isConnected() = 0;

    /**
     * @brief Get current WiFi connection status
     * @return Current WiFi status
     */
    virtual WiFiStatus getStatus() = 0;

    /**
     * @brief Get IP address
     * @param ip_str Buffer to store IP address string (min 16 bytes)
     * @return true if IP address retrieved successfully
     */
    virtual bool getIPAddress(char* ip_str, size_t buffer_size) = 0;

    /**
     * @brief Get MAC address
     * @param mac Buffer to store MAC address (6 bytes)
     * @return true if MAC address retrieved successfully
     */
    virtual bool getMACAddress(uint8_t* mac) = 0;

    /**
     * @brief Get RSSI (signal strength) of current connection
     * @return RSSI in dBm, or 0 if not connected
     */
    virtual int8_t getRSSI() = 0;

    /**
     * @brief Get SSID of current connection
     * @param ssid Buffer to store SSID (min 33 bytes)
     * @return true if SSID retrieved successfully
     */
    virtual bool getSSID(char* ssid, size_t buffer_size) = 0;

    /**
     * @brief Scan for available WiFi networks
     * @param networks Array to store found networks
     * @param max_networks Maximum number of networks to return
     * @param timeout_ms Scan timeout in milliseconds
     * @return Number of networks found
     */
    virtual int32_t scanNetworks(WiFiNetworkInfo* networks, uint8_t max_networks,
                                uint32_t timeout_ms = 10000) = 0;

    /**
     * @brief Enable or disable WiFi power save mode
     * @param enable true to enable power save, false to disable
     * @return true if successful
     */
    virtual bool setPowerSaveMode(bool enable) = 0;

    /**
     * @brief Set WiFi hostname
     * @param hostname Hostname string
     * @return true if successful
     */
    virtual bool setHostname(const char* hostname) = 0;

    /**
     * @brief Test DNS connectivity by resolving a hostname
     * 
     * This is NOT an ICMP ping. It only tests if DNS resolution works,
     * which indicates basic network connectivity.
     * 
     * @param host Hostname or IP address to resolve
     * @param timeout_ms Maximum time to wait for DNS resolution (milliseconds)
     * @return true if DNS resolution succeeded, false otherwise
     * 
     * @note This does NOT guarantee the host is reachable, only that DNS works
     * @note For true ICMP ping, implement using lwIP raw sockets separately
     */
    virtual bool testDNSConnectivity(const char* host, 
                                     uint32_t timeout_ms = 5000) = 0;

    /**
     * @brief Control onboard LED (WiFi chip LED on Pico W)
     * @param state true to turn on, false to turn off
     * @note On Pico W, this controls the CYW43 LED. On other platforms, may control different LED
     */
    virtual void setLED(bool state) = 0;

    /**
     * @brief Enable Access Point mode
     * @param ssid AP SSID
     * @param password AP password (min 8 chars, NULL for open)
     * @param channel WiFi channel (1-11)
     * @return true if AP started successfully
     */
    virtual bool startAP(const char* ssid, const char* password = nullptr,
                         uint8_t channel = 1) = 0;

    /**
     * @brief Disable Access Point mode
     * @return true if AP stopped successfully
     */
    virtual bool stopAP() = 0;
};

#endif // HAL_WIFI_INTERFACE_HPP
