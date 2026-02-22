#include "task_wifi_time_init.hpp"
#include "services/time_service.hpp"
#include "ui/screens/init_status_screen.hpp"
#include "core/state_machine.hpp"
#include "pico/cyw43_arch.h"
#include "wifi_config.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/watchdog.h"
#include <cstdio>

void task_wifi_time_init(void* params) {
    (void)params;
    
    printf("[WiFi-Time] Task started\n");
    
    xEventGroupWaitBits(
        wifi_time_ready_group_event,
        LVGL_READY_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );
    
    init_status_screen_update("Inicializando Sistema", "Preparando hardware WiFi...");
    
    if (cyw43_arch_init()) {
        printf("[WiFi-Time] ERROR: Failed to initialize WiFi!\n");
        init_status_screen_update("Erro de Hardware", "Falha ao inicializar WiFi\nReiniciando em 5s...");
        vTaskDelay(pdMS_TO_TICKS(5000));
        watchdog_reboot(0, 0, 0);
        return;
    }
    
    cyw43_arch_enable_sta_mode();
    
    printf("[WiFi-Time] Connecting to %s...\n", WIFI_SSID);
    init_status_screen_update("Conectando ao WiFi", "Conectando a rede...");
    
    int retry_count = 0;
    const int max_retries = 10;
    bool wifi_connected = false;
    
    printf("[WiFi-Time] Entering WiFi connection loop\n");
    while (retry_count < max_retries) {
        if (retry_count > 0) {
            char retry_msg[64];
            snprintf(retry_msg, sizeof(retry_msg), "Tentativa %d de %d...", retry_count + 1, max_retries);
            init_status_screen_update(nullptr, retry_msg);
        }
        
        int result = cyw43_arch_wifi_connect_timeout_ms(
            WIFI_SSID, 
            WIFI_PASSWORD, 
            CYW43_AUTH_WPA2_AES_PSK, 
            10000
        );
        
        if (result == 0) {
            extern struct netif *netif_default;
            if (netif_default) {
                printf("[WiFi-Time] Connected - IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));
                
                char ip_msg[64];
                snprintf(ip_msg, sizeof(ip_msg), "IP: %s", ip4addr_ntoa(netif_ip4_addr(netif_default)));
                init_status_screen_update("WiFi Conectado!", ip_msg);
                vTaskDelay(pdMS_TO_TICKS(2000));
            
            if (wifi_time_ready_group_event != NULL) {
                xEventGroupSetBits(wifi_time_ready_group_event, WIFI_CONNECTED_BIT);
            }
            
            wifi_connected = true;
            break;
            }
        }
        
        retry_count++;
        printf("[WiFi-Time] Connection attempt %d/%d failed\n", retry_count, max_retries);
    }
    
    if (!wifi_connected) {
        printf("[WiFi-Time] ERROR: Failed after %d attempts\n", max_retries);
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), "Nao foi possivel conectar\napos %d tentativas\nReiniciando em 5s...", max_retries);
        init_status_screen_update("Falha na Conexao WiFi", error_msg);
        vTaskDelay(pdMS_TO_TICKS(5000));
        watchdog_reboot(0, 0, 0);
        return;
    }
    
    init_status_screen_update("Sincronizando Horario", "Obtendo hora do servidor NTP...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    TimeService::getInstance().init();
    
    printf("[WiFi-Time] Waiting for NTP sync...\n");
    int time_wait_count = 0;
    const int max_time_wait = 30;
    
    while (time_wait_count < max_time_wait) {
        if (TimeService::getInstance().isTimeSynced()) {
            printf("[WiFi-Time] Time synchronized\n");
            init_status_screen_update("Horario Sincronizado!", "Sistema pronto para uso");
            vTaskDelay(pdMS_TO_TICKS(2000));
            
            if (wifi_time_ready_group_event != NULL) {
                xEventGroupSetBits(wifi_time_ready_group_event, TIME_SYNCED_BIT);
            }
            break;
        }
        
        if (time_wait_count % 3 == 0) {
            char time_msg[64];
            snprintf(time_msg, sizeof(time_msg), "Aguardando resposta NTP... (%ds)", time_wait_count);
            init_status_screen_update(nullptr, time_msg);  // Keep title
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
        time_wait_count++;
    }
    
    if (time_wait_count >= max_time_wait) {
        printf("[WiFi-Time] ERROR: NTP sync timeout\n");
        init_status_screen_update("Falha na Sincronizacao NTP", "Nao foi possivel sincronizar\no horario\nReiniciando em 5s...");
        vTaskDelay(pdMS_TO_TICKS(5000));
        watchdog_reboot(0, 0, 0);
        return;
    }
    
    printf("[WiFi-Time] Init complete\n");
        
    vTaskDelete(NULL);
}