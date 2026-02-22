#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"

#include "hal/rp2040/hal_wifi_rp2040.hpp"
#include "config/wifi_config.hpp"
#include "config/pin_config.hpp"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"

static HAL_WiFi_RP2040* wifi_hal = nullptr;

// Network info
void displayNetworkInfo() {
    char ip_str[16];
    uint8_t mac[6];
    char ssid[33];
    
    if (wifi_hal->getIPAddress(ip_str, sizeof(ip_str))) {
        printf("IP Address: %s\n", ip_str);
    }
    
    if (wifi_hal->getMACAddress(mac)) {
        printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    
    if (wifi_hal->getSSID(ssid, sizeof(ssid))) {
        printf("SSID: %s\n", ssid);
    }
    
    printf("Signal Strength: %d dBm\n", wifi_hal->getRSSI());
}

// Scan available networks
void scanNetworks() {
    WiFiNetworkInfo networks[10];
    int32_t count = wifi_hal->scanNetworks(networks, 10, 10000);
    
    printf("Found %d network(s)\n\n", count > 0 ? count : 0);
    
    if (count > 0) {
        printf("%-3s %-32s %-4s %-8s %-10s\n", "#", "SSID", "Ch", "Signal", "Security");
        printf("%.3s %.32s %.4s %.8s %.10s\n", "---", "--------------------------------", 
               "----", "--------", "----------");
        
        for (int i = 0; i < count; i++) {
            const char* sec = "???";
            switch (networks[i].security) {
                case WiFiSecurityMode::OPEN: sec = "OPEN"; break;
                case WiFiSecurityMode::WPA_TKIP_PSK: sec = "WPA"; break;
                case WiFiSecurityMode::WPA2_AES_PSK: sec = "WPA2"; break;
                case WiFiSecurityMode::WPA2_MIXED_PSK: sec = "WPA2-MIX"; break;
            }
            
            printf("%-3d %-32s %-4d %4d dBm  %-10s\n",
                   i + 1, networks[i].ssid, networks[i].channel, 
                   networks[i].rssi, sec);
        }
    } else {
        printf("No networks found or scan failed.\n");
    }
}

// Simple HTTP GET (basic example)
void httpGetRequest(const char* host, const char* path) {
    ip_addr_t server_ip;
    
    printf("\nHTTP GET: %s%s\n", host, path);
    
    err_t err = dns_gethostbyname(host, &server_ip, NULL, NULL);
    
    if (err == ERR_OK) {
        printf("IP: %s\n", ip4addr_ntoa(&server_ip));
        
        struct tcp_pcb* pcb = tcp_new();
        if (pcb) {
            tcp_connect(pcb, &server_ip, 80, [](void* arg, struct tcp_pcb* tpcb, err_t err) -> err_t {
                if (err != ERR_OK) {
                    printf("Connect failed: %d\n", err);
                    return err;
                }
                
                const char* req = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
                tcp_write(tpcb, req, strlen(req), TCP_WRITE_FLAG_COPY);
                tcp_output(tpcb);
                
                return ERR_OK;
            });
            
            vTaskDelay(pdMS_TO_TICKS(2000));
            tcp_close(pcb);
        }
    } else {
        printf("DNS failed: %d\n", err);
    }
}

// TCP echo server callbacks
static err_t tcp_server_accept(void* arg, struct tcp_pcb* client_pcb, err_t err) {
    if (err != ERR_OK || !client_pcb) {
        return ERR_VAL;
    }
    
    printf("Client connected\n");
    
    tcp_recv(client_pcb, [](void* arg, struct tcp_pcb* tpcb, struct pbuf* p, err_t err) -> err_t {
        if (!p) {
            printf("Client disconnected\n");
            tcp_close(tpcb);
            return ERR_OK;
        }
        
        // Echo back
        tcp_write(tpcb, p->payload, p->len, TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
        
        pbuf_free(p);
        return ERR_OK;
    });
    
    return ERR_OK;
}

void startTcpEchoServer() {
    struct tcp_pcb* pcb = tcp_new();
    if (!pcb) {
        printf("Failed to create server\n");
        return;
    }
    
    err_t err = tcp_bind(pcb, IP_ADDR_ANY, 7);
    if (err != ERR_OK) {
        printf("Bind failed: %d\n", err);
        tcp_close(pcb);
        return;
    }
    
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, tcp_server_accept);
    
    printf("Echo server on port 7\n");
}

// Main demo task
void wifi_demo_task(void* params) {    
    // Initialize WiFi (must be done after scheduler starts with sys_freertos)
    printf("Step 1: Initializing WiFi...\n");
    printf("Creating WiFi HAL instance...\n");
    wifi_hal = new HAL_WiFi_RP2040();
    
    printf("Initializing CYW43 driver (country: %s)...\n", WIFI_COUNTRY_CODE);
    if (!wifi_hal->init(WIFI_COUNTRY_CODE)) {
        printf("ERROR: WiFi initialization failed!\n");
        vTaskDelete(nullptr);
        return;
    }
    printf("SUCCESS: WiFi initialized\n\n");
    
    // Connect
    printf("Step 2: Connecting to WiFi...\n");
    printf("Network: %s\n", WIFI_SSID);
    printf("Security: WPA2-AES\n");
    printf("Attempting connection (timeout: %d ms)...\n", WIFI_CONNECT_TIMEOUT_MS);
    if (!wifi_hal->connect(WIFI_SSID, WIFI_PASSWORD, 
                          WiFiSecurityMode::WPA2_AES_PSK, 
                          WIFI_CONNECT_TIMEOUT_MS)) {
        printf("ERROR: Connection failed\n");
        vTaskDelete(nullptr);
        return;
    }
    printf("SUCCESS: Connected to network\n\n");
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Step 3: Show network info
    printf("Step 3: Displaying network info...\n");
    printf("Querying network interface...\n");
    displayNetworkInfo();
    printf("SUCCESS: Network info displayed\n\n");
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Step 4: Scan WiFi networks
    printf("Step 4: Scanning WiFi networks...\n");
    printf("Starting active scan (timeout: 10s)...\n");
    scanNetworks();
    printf("SUCCESS: WiFi scan completed\n\n");
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Step 5: DNS test
    printf("Step 5: Testing DNS connectivity...\n");
    printf("Target: google.com\n");
    printf("Resolving hostname...\n");
    if (wifi_hal->testDNSConnectivity("google.com", 5000)) {
        printf("SUCCESS: DNS resolved\n\n");
    } else {
        printf("ERROR: DNS failed\n\n");
    }
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Step 6: Start TCP echo server
    printf("Step 6: Starting TCP echo server...\n");
    printf("Creating TCP server on port 7...\n");
    startTcpEchoServer();
    printf("SUCCESS: Echo server running on port 7\n");
    printf("Usage: telnet <pico-ip> 7\n\n");
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Step 7: Print RSSI samples
    printf("Step 7: Sampling RSSI...\n");
    printf("Collecting 5 signal strength samples (1s interval)...\n");
    for (int i = 0; i < 5; ++i) {
        printf("Sample %d: RSSI = %d dBm\n", i + 1, wifi_hal->getRSSI());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    printf("SUCCESS: RSSI samples complete\n\n");

    printf("Demo completed.\n");
    printf("Cleaning up WiFi HAL...\n");
    delete wifi_hal;
    vTaskDelete(nullptr);
}

// LED blink (uses WiFi HAL LED control)
void blink_task(void* params) {
    // Wait for wifi_hal to be initialized
    while (!wifi_hal) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    while (true) {
        wifi_hal->setLED(true);
        vTaskDelay(pdMS_TO_TICKS(100));
        wifi_hal->setLED(false);
        vTaskDelay(pdMS_TO_TICKS(900));
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);  // Wait for USB serial
    printf("\n\n*****************STARTING********************\n\n");
    
    xTaskCreate(wifi_demo_task, "WiFi_Demo", 4096, nullptr, 2, nullptr);
    xTaskCreate(blink_task, "Blink", 512, nullptr, 1, nullptr);
    
    vTaskStartScheduler();
    
    while (true) {
        tight_loop_contents();
    }
    
    return 0;
}