#include "task_network.hpp"
#include "task_wifi_time_init.hpp"
#include "core/state_machine.hpp"
#include "services/data_storage_service.hpp"
#include "services/network_sync_service.hpp"
#include "services/time_service.hpp"
#include "ui/screens/init_status_screen.hpp"
#include "ui/screens/error_critical_screen.hpp"
#include "config/api_config.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/watchdog.h"
#include <cstdio>

static void show_critical_error_and_reboot(const char* title, const char* message, int seconds = 5)
{
    printf("[Network] CRITICAL ERROR: %s - %s\n", title, message);
    
    error_critical_screen_update(title, message, seconds);
    StateMachine::getInstance().setState(StateMachine::State::ERROR_CRITICAL);
    
    vTaskDelay(pdMS_TO_TICKS(50));
    
    for (int i = seconds - 1; i >= -1; i--) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        error_critical_screen_update_countdown(i);
    }
    
    StateMachine::getInstance().setState(StateMachine::State::RESTARTING);
    vTaskDelay(pdMS_TO_TICKS(200));
    
    watchdog_reboot(0, 0, 0);
}

void task_network(void *params)
{
    (void)params;

    printf("[Network] Task started\n");

    xEventGroupWaitBits(
        wifi_time_ready_group_event,
        WIFI_CONNECTED_BIT | TIME_SYNCED_BIT,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY);

    printf("[Network] WiFi and time ready\n");
    
    init_status_screen_update("Preparando Armazenamento", "Inicializando cartao SD...");
    vTaskDelay(pdMS_TO_TICKS(500));
    
    auto& dataStorage = DataStorageService::getInstance();
    if (!dataStorage.init()) {
        show_critical_error_and_reboot(
            "ERRO NO CARTAO SD",
            "Falha ao montar o cartao SD.\nVerifique o cartao e tente novamente."
        );
        return;
    }
    
    printf("[Network] SD card initialized successfully\n");
    
    StateMachine::getInstance().setState(StateMachine::State::DOWNLOADING_APPOINTMENTS);
    
    init_status_screen_update("Verificando Consultas", "Obtendo data de hoje...");
    vTaskDelay(pdMS_TO_TICKS(500));
    
    std::string today_date = dataStorage.getTodayDate();
    
    if (today_date.empty()) {
        show_critical_error_and_reboot(
            "ERRO DE DATA",
            "Falha ao obter data.\nRTC nao sincronizado.\nVerifique a conexao WiFi."
        );
        return;
    }
    
    printf("[Network] Today's date: %s\n", today_date.c_str());
    
    auto& networkSync = NetworkSyncService::getInstance();
    
    if (!networkSync.hasToken()) {
        printf("[Network] Authenticating with API...\n");
        init_status_screen_update("Autenticando", "Conectando com servidor...");
        vTaskDelay(pdMS_TO_TICKS(500));
        
        if (!networkSync.authenticate(API_AUTH_TIMEOUT_MS)) {
            show_critical_error_and_reboot(
                "ERRO DE AUTENTICACAO",
                "Falha ao autenticar com API.\nVerifique credenciais e conexao."
            );
            return;
        }
        
        printf("[Network] Authentication successful\n");
        init_status_screen_update("Autenticado", "Conexao estabelecida!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    if (dataStorage.hasAppointments(today_date)) {
        printf("[Network] Appointments already downloaded for %s\n", today_date.c_str());
        init_status_screen_update("Consultas Encontradas", "Consultas ja foram baixadas");
        vTaskDelay(pdMS_TO_TICKS(1500));
    } else {
        printf("[Network] Downloading appointments for %s...\n", today_date.c_str());
        
        char download_msg[64];
        snprintf(download_msg, sizeof(download_msg), "Baixando consultas de %s...", today_date.c_str());
        init_status_screen_update("Sincronizando Consultas", download_msg);
        vTaskDelay(pdMS_TO_TICKS(500));
        
        std::string json_response;
        
        if (networkSync.downloadAppointments(today_date, json_response, 30000)) {
            init_status_screen_update("Salvando Consultas", "Gravando dados no cartao SD...");
            vTaskDelay(pdMS_TO_TICKS(500));
            
            if (dataStorage.saveAppointments(today_date, json_response)) {
                printf("[Network] Appointments downloaded and saved\n");
                init_status_screen_update("Consultas Salvas", "Dados gravados com sucesso!");
                vTaskDelay(pdMS_TO_TICKS(1500));
                
                init_status_screen_update("Digitais", "Baixando templates biometricos...");
                vTaskDelay(pdMS_TO_TICKS(500));
                
                int fingerprints_downloaded = networkSync.downloadFingerprintsForAppointments(json_response);
                printf("[Network] Downloaded %d fingerprint templates\n", fingerprints_downloaded);
                
                if (fingerprints_downloaded > 0) {
                    char msg[64];
                    snprintf(msg, sizeof(msg), "%d templates baixados", fingerprints_downloaded);
                    init_status_screen_update("Digitais OK", msg);
                    vTaskDelay(pdMS_TO_TICKS(1500));
                }
            } else {
                show_critical_error_and_reboot(
                    "ERRO AO SALVAR",
                    "Falha ao gravar consultas.\nCartao SD cheio ou corrompido."
                );
                return;
            }
        } else {
            show_critical_error_and_reboot(
                "ERRO NO DOWNLOAD",
                "Falha ao baixar consultas da API.\nVerifique a conexao de rede."
            );
            return;
        }
    }
    
    printf("[Network] System ready for operation\n");
    init_status_screen_update("Sistema Pronto", "Aguardando pacientes...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    StateMachine::getInstance().setState(StateMachine::State::IDLE);
    printf("[Network] State set to IDLE - system ready for operation\n");
    
    std::string last_checked_date = dataStorage.getTodayDate();
    printf("[Network] Date monitor started (%s)\n", last_checked_date.c_str());
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(60000));
        
        std::string current_date = dataStorage.getTodayDate();
        
        if (current_date != last_checked_date) {
            printf("[Network] Date changed: %s -> %s\n", 
                   last_checked_date.c_str(), current_date.c_str());
            
            if (!dataStorage.hasAppointments(current_date)) {
                std::string json_response;
                auto& networkSync = NetworkSyncService::getInstance();
                
                if (networkSync.downloadAppointments(current_date, json_response, 30000)) {
                    if (dataStorage.saveAppointments(current_date, json_response)) {
                        printf("[Network] New date appointments saved\n");
                        
                        int fingerprints_downloaded = networkSync.downloadFingerprintsForAppointments(json_response);
                        printf("[Network] Fingerprints for new date: %d\n", fingerprints_downloaded);
                    } else {
                        printf("[Network] WARNING: Failed to save appointments for new date\n");
                    }
                } else {
                    printf("[Network] WARNING: Failed to download appointments for new date\n");
                }
            }
            
            last_checked_date = current_date;
        }
    }
}