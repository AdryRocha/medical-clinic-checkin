#include "core/checkin_state_machine.hpp"
#include "services/logger_service.hpp"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "ff.h"
#include <cstring> // Necessário para memset e strncpy

SemaphoreHandle_t g_sm_mutex = nullptr;

CheckinStateMachine::CheckinStateMachine()
    : state_(CheckinState::INITIALIZING)

    // Queue agora aloca o tamanho da estrutura EventMessage
    event_queue = xQueueCreate(10, sizeof(EventMessage));
StateMachine& StateMachine::getInstance() {
    static StateMachine instance;
    return instance;
}

void StateMachine::setState(State newState) {
    state_ = newState;
}

StateMachine::State StateMachine::getState() const {
    return state_;
}

const char* StateMachine::getStateName() const {
    return getStateName(state_);
}

const char* StateMachine::getStateName(State state) const {
    switch (state) {
        case State::INITIALIZING:            return "INITIALIZING";
        case State::DOWNLOADING_APPOINTMENTS: return "DOWNLOADING_APPOINTMENTS";
        case State::IDLE:                    return "IDLE";
        case State::VALIDATING:              return "VALIDATING";
        case State::FINGERPRINT_VERIFYING:   return "FINGERPRINT_VERIFYING";
        case State::FINGERPRINT_ENROLLING:   return "FINGERPRINT_ENROLLING";
        case State::FINGERPRINT_UPLOADING:   return "FINGERPRINT_UPLOADING";
        case State::APPOINTMENT:             return "APPOINTMENT";
        case State::ERROR:                   return "ERROR";
        case State::ERROR_CRITICAL:          return "ERROR_CRITICAL";
        case State::RESTARTING:              return "RESTARTING";
        default:                             return "UNKNOWN";
    }
}

void CheckinStateMachine::process_event(CheckinEvent event, const char* payload) {
    EventMessage msg;
    msg.type = event;
    
    // Limpa o buffer e garante o terminador nulo
    memset(msg.data, 0, sizeof(msg.data));
    if (payload != nullptr) {
        // Copia no máximo 63 caracteres para deixar espaço para o '\0'
        strncpy(msg.data, payload, sizeof(msg.data) - 1);
        msg.data[sizeof(msg.data) - 1] = '\0'; 
    }

    Logger::info("[FSM] Enfileirando evento: %d", (int)event);
    xQueueSend(event_queue, &msg, 0);
}

void CheckinStateMachine::process_loop() {
    EventMessage msg; // Variável para receber da fila

    while (true) {
        // Recebe a estrutura completa (msg)
        if (xQueueReceive(event_queue, &msg, portMAX_DELAY) == pdTRUE) {
            Logger::info("[FSM] Recebido evento da fila: %d", (int)msg.type);

            if (g_sm_mutex) xSemaphoreTake(g_sm_mutex, portMAX_DELAY);

            CheckinState prev = state_;
            CheckinEvent event = msg.type; // Extrai o evento
            const char* data = msg.data;   // Extrai os dados (Agora 'data' existe!)

            switch (state_) {
                case CheckinState::AGUARDANDO_QR:
                    if (event == CheckinEvent::QR_CODE_LIDO) {
                        state_ = CheckinState::PROCESSANDO_DADOS_PACIENTE;
                        Logger::info("[FSM] QR Lido: Processando %s", data);
                    } else if (event == CheckinEvent::INICIAR_CHECKIN) {
                        state_ = CheckinState::REGISTRO_INICIADO;
                    }
                    break;

                case CheckinState::PROCESSANDO_DADOS_PACIENTE:
                    if (event == CheckinEvent::PACIENTE_ENCONTRADO) state_ = CheckinState::VALIDACAO_BIOMETRICA;
                    else if (event == CheckinEvent::PACIENTE_NAO_ENCONTRADO) state_ = CheckinState::REGISTRO_INICIADO;
                    else state_ = CheckinState::EM_ERRO;
                    break;

                case CheckinState::REGISTRO_INICIADO:
                    state_ = CheckinState::COLETANDO_DIGITAL_CADASTRO;
                    break;

                case CheckinState::COLETANDO_DIGITAL_CADASTRO:
                    if (event == CheckinEvent::ARQUIVO_SALVO_SUCESSO) {
                        state_ = CheckinState::CADASTRO_CONCLUIDO;
                        FIL file;
                        if (f_open(&file, "cadastros.txt", FA_WRITE | FA_OPEN_APPEND) == FR_OK) {
                            f_printf(&file, "Cadastro: %s\n", data);
                            f_close(&file);
                            Logger::info("[FSM] Dados salvos no SD");
                        }
                    } else if (event == CheckinEvent::ERRO_GENERICO) state_ = CheckinState::EM_ERRO;
                    break;

                case CheckinState::VALIDACAO_BIOMETRICA:
                    if (event == CheckinEvent::DIGITAL_MATCH_OK) {
                        state_ = CheckinState::CONSULTA_CONFIRMADA;
                        FIL file;
                        if (f_open(&file, "checkins.txt", FA_WRITE | FA_OPEN_APPEND) == FR_OK) {
                            f_printf(&file, "Confirmado: %s\n", data);
                            f_close(&file);
                        }
                    } else if (event == CheckinEvent::DIGITAL_MATCH_FAIL) state_ = CheckinState::EM_ERRO;
                    break;

                default:
                    break;
            }

            if (event == CheckinEvent::TIMEOUT_INTERFACE) state_ = CheckinState::AGUARDANDO_QR;

            if (state_ != prev) Logger::info("[FSM] Transição: %s -> %s", get_state_name(prev), get_state_name(state_));

            if (g_sm_mutex) xSemaphoreGive(g_sm_mutex);
        }
    }
}