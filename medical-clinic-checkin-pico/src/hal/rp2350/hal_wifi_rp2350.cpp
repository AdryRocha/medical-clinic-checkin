#include "hal/rp2350/hal_wifi_rp2350.hpp"
#include "pico/cyw43_arch.h"
#include "services/logger_service.hpp"
#include "lwip/ip4_addr.h"
#include "FreeRTOS.h"
#include "task.h"

HAL_WIFI_RP2350::HAL_WIFI_RP2350() {
}

bool HAL_WIFI_RP2350::init(const char* country_code) {
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_BRAZIL)) {
        Logger::error("[WIFI] Falha ao inicializar cyw43_arch");
        return false;
    }
    cyw43_arch_enable_sta_mode();
    
    // Desabilitar Power Save para evitar desconexões aleatórias
    cyw43_wifi_pm(&cyw43_state, cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 20, 1, 1, 1));

    Logger::info("[WIFI] Inicializado (Country: %s)", country_code);
    return true;
}

void HAL_WIFI_RP2350::deinit() {
    cyw43_arch_deinit();
}

bool HAL_WIFI_RP2350::connect(const char* ssid, const char* pwd, WiFiSecurityMode sec, uint32_t timeout_ms) {
    // Loop de Retentativa (Retry Logic)
    const int MAX_RETRIES = 3;
    
    uint32_t auth_type;
    switch(sec) {
        case WiFiSecurityMode::OPEN: auth_type = CYW43_AUTH_OPEN; break;
        case WiFiSecurityMode::WPA2: auth_type = CYW43_AUTH_WPA2_AES_PSK; break;
        case WiFiSecurityMode::WPA3: auth_type = CYW43_AUTH_WPA3_WPA2_AES_PSK; break;
        default: auth_type = CYW43_AUTH_WPA2_AES_PSK; break;
    }

    for (int i = 0; i < MAX_RETRIES; i++) {
        Logger::info("[WIFI] Tentativa %d/%d conectando em %s...", i+1, MAX_RETRIES, ssid);
        
        int err = cyw43_arch_wifi_connect_timeout_ms(ssid, pwd, auth_type, timeout_ms);
        
        if (err == 0) {
            Logger::info("[WIFI] Conectado com Sucesso!");
            return true;
        }
        
        Logger::error("[WIFI] Falha (%d). Tentando novamente em 1s...", err);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    Logger::error("[WIFI] Desistindo apos %d tentativas.", MAX_RETRIES);
    return false;
}

void HAL_WIFI_RP2350::disconnect() {
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
}

WiFiStatus HAL_WIFI_RP2350::getStatus() {
    int status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    switch(status) {
        case CYW43_LINK_UP: return WiFiStatus::CONNECTED;
        case CYW43_LINK_DOWN: return WiFiStatus::DISCONNECTED;
        case CYW43_LINK_JOIN: return WiFiStatus::CONNECTING;
        case CYW43_LINK_NOIP: return WiFiStatus::CONNECTING;
        default: return WiFiStatus::ERROR;
    }
}

bool HAL_WIFI_RP2350::isConnected() {
    return getStatus() == WiFiStatus::CONNECTED;
}

bool HAL_WIFI_RP2350::getIPAddress(char* ip_str, size_t size) {
    if (!isConnected()) return false;
    const ip4_addr_t* ip = netif_ip4_addr(netif_list);
    ip4addr_ntoa_r(ip, ip_str, size);
    return true;
}

bool HAL_WIFI_RP2350::getMACAddress(uint8_t* mac) {
    return false; 
}