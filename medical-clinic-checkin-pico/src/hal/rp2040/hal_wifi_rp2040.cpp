#include "hal/rp2040/hal_wifi_rp2040.hpp"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>

// -------------------------
// lwIP (Pico SDK 2.1.1)
// -------------------------
extern "C" {
#include "lwip/opt.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"
#include "lwip/apps/sntp.h"
#include "lwip/timeouts.h"
}

// Remove macro conflitante
#ifdef connect
#undef connect
#endif

// ============================================================
// CTOR / DTOR
// ============================================================

HAL_WiFi_RP2040::HAL_WiFi_RP2040()
    : initialized_(false),
      status_(WiFiStatus::DISCONNECTED)
{
}

HAL_WiFi_RP2040::~HAL_WiFi_RP2040()
{
    deinit();
}

// ============================================================
// INIT / DEINIT
// ============================================================

bool HAL_WiFi_RP2040::init(const char* country_code)
{
    (void)country_code; 

    if (initialized_) return true;

    printf("[WiFi] cyw43_arch_init()...\n");

    if (cyw43_arch_init() != 0) {
        status_ = WiFiStatus::ERROR;
        return false;
    }

    cyw43_arch_enable_sta_mode();

    initialized_ = true;
    status_ = WiFiStatus::DISCONNECTED;

    printf("[WiFi] STA mode OK\n");
    return true;
}

void HAL_WiFi_RP2040::deinit()
{
    if (!initialized_) return; // Corrigido: initialized_

    disconnect();

    cyw43_arch_deinit();
    initialized_ = false;      // Corrigido: initialized_
    status_ = WiFiStatus::DISCONNECTED; // Corrigido: status_

    printf("[WiFi] deinit OK\n");
}

// ============================================================
// CONNECT / DISCONNECT
// ============================================================

bool HAL_WiFi_RP2040::connect(const char* ssid,
                             const char* password,
                             WiFiSecurityMode sec,
                             uint32_t timeout_ms)
{
    if (!initialized_) { // Corrigido: initialized_
        printf("[WiFi] ERRO: init() não foi chamado\n");
        status_ = WiFiStatus::ERROR; // Corrigido: status_
        return false;
    }

    if (!ssid || ssid[0] == '\0') {
        printf("[WiFi] ERRO: SSID vazio\n");
        status_ = WiFiStatus::ERROR; // Corrigido: status_
        return false;
    }

    status_ = WiFiStatus::CONNECTING; // Corrigido: status_
    
    // --- LÓGICA DE CONEXÃO ---
    
    uint32_t auth = CYW43_AUTH_OPEN;
    switch (sec) {
        case WiFiSecurityMode::OPEN: auth = CYW43_AUTH_OPEN; break;
        case WiFiSecurityMode::WPA:  auth = CYW43_AUTH_WPA_TKIP_PSK; break;
        case WiFiSecurityMode::WPA2: auth = CYW43_AUTH_WPA2_AES_PSK; break;
        case WiFiSecurityMode::WPA3: auth = CYW43_AUTH_WPA2_AES_PSK; break; // Fallback WPA2
        default: auth = CYW43_AUTH_WPA2_AES_PSK; break;
    }

    const char* pass = (password ? password : "");

    printf("[WiFi] Conectando SSID='%s' (timeout=%u ms)\n", ssid, (unsigned)timeout_ms);

    int rc = cyw43_arch_wifi_connect_timeout_ms(ssid, pass, auth, timeout_ms);

    if (rc == 0) {
        status_ = WiFiStatus::CONNECTED; // Corrigido: status_
        printf("[WiFi] Conectado!\n");
        return true;
    }

    status_ = WiFiStatus::DISCONNECTED; // Corrigido: status_
    printf("[WiFi] Falha ao conectar rc=%d\n", rc);
    return false;
}

void HAL_WiFi_RP2040::disconnect()
{
    if (!initialized_) return; // Corrigido: initialized_

    cyw43_wifi_set_up(&cyw43_state, CYW43_ITF_STA, false, 0);
    sleep_ms(50);
    cyw43_wifi_set_up(&cyw43_state, CYW43_ITF_STA, true, 0);

    status_ = WiFiStatus::DISCONNECTED; // Corrigido: status_
    printf("[WiFi] Disconnected\n");
}

// ============================================================
// STATUS / INFO
// ============================================================

WiFiStatus HAL_WiFi_RP2040::getStatus()
{
    return status_; // Corrigido: status_
}

bool HAL_WiFi_RP2040::getIPAddress(char* ip_str, size_t size)
{
    if (!ip_str || size == 0) return false;

    ip4_addr_t ip = cyw43_state.netif[CYW43_ITF_STA].ip_addr;
    if (ip.addr == 0) return false;

    snprintf(ip_str, size, "%s", ip4addr_ntoa(&ip));
    return true;
}

bool HAL_WiFi_RP2040::getMACAddress(uint8_t* mac)
{
    if (!mac) return false;
    memcpy(mac, cyw43_state.mac, 6);
    return true;
}

int8_t HAL_WiFi_RP2040::getRSSI()
{
    if (status_ != WiFiStatus::CONNECTED) return -127; // Corrigido: status_

    int32_t rssi = -127;
    if (cyw43_wifi_get_rssi(&cyw43_state, &rssi) == 0) {
        return (int8_t)rssi;
    }
    return -127;
}

bool HAL_WiFi_RP2040::getSSID(char* ssid, size_t size)
{
    if (!ssid || size == 0) return false;
    ssid[0] = '\0';
    return true;
}

// ============================================================
// PING
// ============================================================

bool HAL_WiFi_RP2040::ping(const char* host, int attempts, uint32_t timeout_ms)
{
    if (status_ != WiFiStatus::CONNECTED) return false; // Corrigido: status_
    if (!host || host[0] == '\0') return false;
    if (attempts <= 0) attempts = 1;

    printf("[WiFi] ping(tcp) host='%s' attempts=%d timeout=%u ms\n",
           host, attempts, (unsigned)timeout_ms);

    struct hostent* he = nullptr;

    cyw43_arch_lwip_begin();
    he = lwip_gethostbyname(host);
    cyw43_arch_lwip_end();

    if (!he || !he->h_addr_list || !he->h_addr_list[0]) {
        printf("[WiFi] DNS falhou para '%s'\n", host);
        return false;
    }

    struct sockaddr_in target;
    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port   = PP_HTONS(80);
    memcpy(&target.sin_addr, he->h_addr_list[0], (size_t)he->h_length);

    struct timeval tv;
    tv.tv_sec  = (long)(timeout_ms / 1000);
    tv.tv_usec = (long)((timeout_ms % 1000) * 1000);

    while (attempts-- > 0)
    {
        int sock = lwip_socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            printf("[WiFi] socket() falhou\n");
            return false;
        }

        lwip_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        lwip_setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        int rc = lwip_connect(sock, (struct sockaddr*)&target, sizeof(target));
        lwip_close(sock);

        if (rc == 0) {
            printf("[WiFi] ping(tcp) OK\n");
            return true;
        }

        printf("[WiFi] ping(tcp) falhou, restam %d\n", attempts);
        sleep_ms(150);
    }

    return false;
}

// ============================================================
// NTP
// ============================================================

bool HAL_WiFi_RP2040::syncTimeNTP(const char* server, uint32_t timeout_ms)
{
    if (status_ != WiFiStatus::CONNECTED) return false; // Corrigido: status_
    if (!server || server[0] == '\0') return false;

    printf("[WiFi] NTP sync server='%s' timeout=%u ms\n",
           server, (unsigned)timeout_ms);

    ip_addr_t ntp_ip;
    memset(&ntp_ip, 0, sizeof(ntp_ip));

    err_t err;

    cyw43_arch_lwip_begin();
    err = dns_gethostbyname(server, &ntp_ip, nullptr, nullptr);
    cyw43_arch_lwip_end();

    if (err != ERR_OK) {
        printf("[WiFi] DNS NTP falhou err=%d\n", (int)err);
        return false;
    }

    cyw43_arch_lwip_begin();
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setserver(0, &ntp_ip);
    sntp_init();
    cyw43_arch_lwip_end();

    absolute_time_t start = get_absolute_time();

    while (absolute_time_diff_us(start, get_absolute_time()) < (int64_t)timeout_ms * 1000) {
        cyw43_arch_lwip_begin();
        sys_check_timeouts();
        cyw43_arch_lwip_end();
        sleep_ms(200);
    }

    printf("[WiFi] NTP solicitado (best-effort)\n");
    return true;
}