#include "tasks/task_qr.hpp"
#include "services/logger_service.hpp"
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string>
#include <cstring>

static char qr_buffer[128];
static int qr_pos = 0;

TaskQR::TaskQR(HAL_UART_Interface* uart, CheckinStateMachine* fsm)
    : uart_(uart), fsm_(fsm)
{
}

void TaskQR::run() {
    Logger::info("[QR] Tarefa Iniciada. Aguardando Leitura...");

    TaskHandle_t this_task = xTaskGetCurrentTaskHandle();
    while (true) {
        uint8_t c = 0;
        int bytes_read = uart_->read(&c, 1);

        if (bytes_read > 0) {
            if (c == '\r' || c == '\n') {
                if (qr_pos > 0) {
                    qr_buffer[qr_pos] = '\0';
                    // CORREÇÃO: Validação simples (ex.: comprimento)
                    if (strlen(qr_buffer) > 5) { // Exemplo mínimo para QR real
                        Logger::info("[QR] Codigo Lido: %s", qr_buffer);
                        if (fsm_) {
                            Logger::info("[QR] Enfileirando evento QR_CODE_LIDO");
                            fsm_->process_event(CheckinEvent::QR_CODE_LIDO, qr_buffer);

                            // Caminho curto: simula paciente encontrado e digital OK
                            Logger::info("[QR] Simulando PACIENTE_ENCONTRADO -> DIGITAL_MATCH_OK");
                            fsm_->process_event(CheckinEvent::PACIENTE_ENCONTRADO, qr_buffer);
                            fsm_->process_event(CheckinEvent::DIGITAL_MATCH_OK, qr_buffer);
                        }
                    } else {
                        Logger::warn("[QR] Código inválido (curto)");
                    }
                    qr_pos = 0;
                    memset(qr_buffer, 0, sizeof(qr_buffer));
                }
            } else if (qr_pos < sizeof(qr_buffer) - 1) {
                qr_buffer[qr_pos++] = (char)c;
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(20)); // CORREÇÃO: Delay reduzido para responsividade
        }

        // Log periódico de stack e heap
        static TickType_t last_log = 0;
        if ((xTaskGetTickCount() - last_log) > pdMS_TO_TICKS(2000)) {
            UBaseType_t stack_min = uxTaskGetStackHighWaterMark(this_task);
            Logger::info("[QR] loop ativo, heap livre: %u, stack min: %u words", xPortGetFreeHeapSize(), (unsigned)stack_min);
            last_log = xTaskGetTickCount();
        }
    }
}