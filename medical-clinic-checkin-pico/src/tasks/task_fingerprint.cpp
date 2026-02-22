#include "tasks/task_fingerprint.hpp"
#include "services/logger_service.hpp"
#include "core/checkin_state_machine.hpp" // Novo caminho

TaskFingerprint::TaskFingerprint(HAL_UART_Interface* uart, RingBuffer* buffer)
    : uart_(uart), buffer_(buffer)
{
    // CORREÇÃO: Removido o 'new RingBuffer'. 
    // O buffer agora é injetado via BoardFactory (estático).
}

void TaskFingerprint::run() {
    Logger::info("[Fingerprint] Task iniciada com buffer estático.");

    uint8_t rx;
    for (;;)
    {
        // Usa read_byte ou readByte conforme a interface
        if (uart_->available() > 0)
        {
            rx = uart_->readByte();
            // buffer_->push(rx); // Se usar ring buffer local
            
            // Lógica simplificada:
            // if (buffer_->size() >= 12 && decodePacket()) buffer_->clear();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

bool TaskFingerprint::decodePacket() {
    uint8_t header_high, header_low;
    
    // 1. Verifica o cabeçalho padrão do R307S (0xEF01)
    if (buffer_->pop(header_high) && buffer_->pop(header_low)) {
        if (header_high == 0xEF && header_low == 0x01) {
            
            // Pula 4 bytes de endereço (0xFFFFFFFF) e 1 byte de ID de pacote
            for(int i = 0; i < 5; i++) { uint8_t dummy; buffer_->pop(dummy); }
            
            uint8_t len_high, len_low, confirm_code;
            buffer_->pop(len_high); // Comprimento
            buffer_->pop(len_low);
            buffer_->pop(confirm_code); // Código de Confirmação (0x00 = Sucesso)

            if (confirm_code == 0x00) {
                Logger::info("[FP] Digital reconhecida com sucesso!");
                CheckinStateMachine::instance().process_event(CheckinEvent::DIGITAL_MATCH_OK);
                return true;
            } else {
                Logger::warn("[FP] Digital não corresponde. Código: %02X", confirm_code);
                CheckinStateMachine::instance().process_event(CheckinEvent::DIGITAL_MATCH_FAIL);
            }
        }
    }
    return false;
}