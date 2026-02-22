#include "tasks/task_qr_validator.hpp"
#include "core/state_machine.hpp"
#include "services/appointment_service.hpp"
#include "services/fingerprint_service.hpp"
#include "services/data_storage_service.hpp"
#include "services/network_sync_service.hpp"
#include "ui/screens/appointment_screen.hpp"
#include "ui/screens/error_screen.hpp"
#include "ui/screens/fingerprint_operation_screen.hpp"
#include "ui/screens/fingerprint_upload_screen.hpp"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>

#define QR_BUFFER_SIZE 512
#define DISPLAY_TIME_MS 5000

#define SIMULATION_MODE 1

void task_qr_validator(void* params) {
    QRValidatorParams* validator_params = static_cast<QRValidatorParams*>(params);
    
    if (!validator_params || !validator_params->qr_data_queue) {
        printf("[QR Validator] ERROR: Invalid parameters!\n");
        vTaskDelete(NULL);
        return;
    }
    
    QueueHandle_t qr_data_queue = validator_params->qr_data_queue;
    
    printf("[QR Validator] Started\n");
    
    char qr_data[QR_BUFFER_SIZE];
    
    while (1) {
        if (xQueueReceive(qr_data_queue, qr_data, portMAX_DELAY) == pdTRUE) {
            
            printf("[QR Validator] Received data: %s\n", qr_data);
            
            auto& appointmentService = AppointmentService::getInstance();
            
            QRCodeData qr_code;
            if (!appointmentService.validateAndParseQRCode(qr_data, qr_code)) {
                printf("[QR Validator] ERROR: Failed to parse QR code!\n");
                
                error_screen_update(
                    "QR Code invalido",
                    "Nao foi possivel ler o codigo", "Tente novamente"
                );
                StateMachine::getInstance().setState(StateMachine::State::ERROR);
                vTaskDelay(pdMS_TO_TICKS(3000));
                StateMachine::getInstance().setState(StateMachine::State::IDLE);
                continue;
            }
            
            printf("[QR Validator] QR parsed - ID: %d, Name: %s\n",
                   qr_code.appt_id, qr_code.name.c_str());
            
            AppointmentInfo appointment;
            if (!appointmentService.validateAppointmentById(qr_code, appointment)) {
                printf("[QR Validator] ERROR: Appointment not found or invalid!\n");
                
                if (appointment.found && appointment.status == "realizada") {
                    printf("[QR Validator] Appointment already completed\n");
                    
                    error_screen_update(
                        "Check-in ja realizado",
                        "Sua consulta ja foi registrada. Aguarde ser chamado pela recepcionista",
                        "Obrigado!"
                    );
                } else {
                    std::string masked_cpf = AppointmentService::maskCPF(qr_code.cpf);
                    char message[256];
                    snprintf(message, sizeof(message), "Ola %s, CPF %s, nao encontramos sua consulta nos agendamentos de hoje", 
                             qr_code.name.c_str(), masked_cpf.c_str());
                    
                    error_screen_update(
                        "Consulta nao encontrada",
                        message,
                        "Entre em contato com a recepcao"
                    );
                }
                
                StateMachine::getInstance().setState(StateMachine::State::ERROR);
                
                vTaskDelay(pdMS_TO_TICKS(5000));
                StateMachine::getInstance().setState(StateMachine::State::IDLE);
                continue;
            }
            
            printf("[QR Validator] Appointment validated: %s\n", appointment.patient_name.c_str());
            
            if (appointment.requires_fingerprint_verification) {
                StateMachine::getInstance().setState(StateMachine::State::FINGERPRINT_VERIFYING);
                
                fingerprint_operation_screen_show(FINGERPRINT_VERIFY);
                fingerprint_operation_screen_update_status("Aguardando digital...");
                vTaskDelay(pdMS_TO_TICKS(500));
                
                auto& fingerprintService = FingerprintService::getInstance();
                
                // fingerprint_operation_screen_update_status("Coloque seu dedo no sensor...");
                // uint16_t confidence = 0;
                // bool verified = fingerprintService.verifyFingerprint(appointment.patient.id, confidence, 15000);
                
                fingerprint_operation_screen_update_status("Coloque seu dedo no sensor...");

                uint16_t confidence = 0;
                bool verified = false;

                #if SIMULATION_MODE
                printf("[SIM] Skipping fingerprint verification\n");
                confidence = 100;  // só pra log
                verified = true;   // força sucesso
                vTaskDelay(pdMS_TO_TICKS(800)); // opcional: dá tempo de ver a tela
                #else
                verified = fingerprintService.verifyFingerprint(appointment.patient.id, confidence, 15000);
                #endif


                if (!verified) {
                    
                    error_screen_update(
                        "Digital Nao Reconhecida",
                        "Sua digital nao foi reconhecida. Entre em contato com a recepcao",
                        "Tente novamente"
                    );
                    StateMachine::getInstance().setState(StateMachine::State::ERROR);
                    vTaskDelay(pdMS_TO_TICKS(5000));
                    StateMachine::getInstance().setState(StateMachine::State::IDLE);
                    continue;
                }
                
                printf("[QR Validator] Fingerprint verified (confidence: %d)\n", confidence);
                fingerprint_operation_screen_update_status("Digital verificada com sucesso!");
                vTaskDelay(pdMS_TO_TICKS(1500));
            }
            
            if (appointment.requires_fingerprint_enrollment) {
                StateMachine::getInstance().setState(StateMachine::State::FINGERPRINT_ENROLLING);
                
                fingerprint_operation_screen_show(FINGERPRINT_ENROLL);
                fingerprint_operation_screen_set_step(1);
                fingerprint_operation_screen_update_status("Aguardando primeira leitura...");
                vTaskDelay(pdMS_TO_TICKS(500));
                
                auto& fingerprintService = FingerprintService::getInstance();
                auto& dataStorage = DataStorageService::getInstance();
                
                std::vector<uint8_t> template_data;
                bool enrolled = fingerprintService.enrollFingerprint(template_data, 30000);
                
                if (!enrolled) {
                    
                    error_screen_update(
                        "Falha no Cadastro",
                        "Nao foi possivel cadastrar sua digital. Entre em contato com a recepcao",
                        "Tente novamente"
                    );
                    StateMachine::getInstance().setState(StateMachine::State::ERROR);
                    vTaskDelay(pdMS_TO_TICKS(5000));
                    StateMachine::getInstance().setState(StateMachine::State::IDLE);
                    continue;
                }
                
                printf("[QR Validator] Fingerprint enrolled (%zu bytes)\n", template_data.size());
                fingerprint_operation_screen_update_status("Digital cadastrada com sucesso!");
                vTaskDelay(pdMS_TO_TICKS(1500));
                
                if (!dataStorage.saveFingerprintTemplate(appointment.patient.id, template_data)) {
                    printf("[QR Validator] ERROR: Failed to save template\n");
                    
                    error_screen_update(
                        "Erro ao Salvar",
                        "Sua digital foi capturada mas nao foi possivel salvar. Entre em contato com a recepcao",
                        ""
                    );
                    StateMachine::getInstance().setState(StateMachine::State::ERROR);
                    vTaskDelay(pdMS_TO_TICKS(5000));
                    StateMachine::getInstance().setState(StateMachine::State::IDLE);
                    continue;
                }
                
                printf("[QR Validator] Template saved\n");
                
                StateMachine::getInstance().setState(StateMachine::State::FINGERPRINT_UPLOADING);
                
                fingerprint_upload_screen_show();
                fingerprint_upload_screen_update_status("Sincronizando com servidor...");
                vTaskDelay(pdMS_TO_TICKS(500));
                
                auto& networkSync = NetworkSyncService::getInstance();
                bool uploaded = networkSync.uploadFingerprintTemplate(appointment.patient.id, template_data, 30000);
                
                if (!uploaded) {
                    printf("[QR Validator] WARNING: Upload failed (will retry later)\n");
                    fingerprint_upload_screen_update_status("Upload offline - sera sincronizado depois");
                    vTaskDelay(pdMS_TO_TICKS(2000));
    
                } else {
                    printf("[QR Validator] Fingerprint uploaded\n");
                    fingerprint_upload_screen_update_status("Digital enviada com sucesso!");
                    vTaskDelay(pdMS_TO_TICKS(1500));
                }
            }
            
            char success_patient[100];
            snprintf(success_patient, sizeof(success_patient), "%s", appointment.patient_name.c_str());
            
            char success_cpf[50];
            snprintf(success_cpf, sizeof(success_cpf), "%s", appointment.patient_cpf.c_str());
            
            char success_time[50];
            snprintf(success_time, sizeof(success_time), "Horario: %s", appointment.time.c_str());
            
            char success_info[100];
            snprintf(success_info, sizeof(success_info), "%s - %s", 
                     appointment.professional_name.c_str(), 
                     appointment.category.c_str());
            
            appointment_screen_update(
                success_patient,
                success_cpf,
                success_time,
                success_info
            );
            
            StateMachine::getInstance().setState(StateMachine::State::APPOINTMENT);
            
            printf("[QR Validator] Marking appointment %d completed\n", appointment.id);
            if (appointmentService.markAppointmentCompleted(appointment.id)) {
                printf("[QR Validator] Check-in complete\n");
            } else {
                printf("[QR Validator] WARNING: Failed to mark completed\n");
            }
            
            vTaskDelay(pdMS_TO_TICKS(DISPLAY_TIME_MS));
            
            StateMachine::getInstance().setState(StateMachine::State::IDLE);
        }
    }
}