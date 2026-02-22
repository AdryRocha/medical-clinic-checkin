#pragma once
#include "FreeRTOS.h"
#include "queue.h"
#include <string>

class CheckinEvent {
public:
    enum class State {
        INITIALIZING,             // WiFi + time initialization
        DOWNLOADING_APPOINTMENTS, // Downloading appointments from API
        IDLE,                     // Waiting for QR scan
        VALIDATING,               // Processing JSON data
        FINGERPRINT_VERIFYING,    // Verifying patient fingerprint
        FINGERPRINT_ENROLLING,    // Enrolling new fingerprint
        FINGERPRINT_UPLOADING,    // Uploading fingerprint to API
        APPOINTMENT,              // Displaying appointment info
        ERROR,                    // QR code read/validation error
        ERROR_CRITICAL,           // Critical system error
        RESTARTING                // System restarting
    };

// Estrutura para trafegar dados na Queue
struct EventMessage {
    CheckinEvent type;
    char data[64]; // Buffer para guardar strings (QR Code, ID, etc)
};

class CheckinStateMachine {
public:
    static CheckinStateMachine& instance() {
        static CheckinStateMachine instance;
        return instance;
    }

    CheckinStateMachine();
    void process_event(CheckinEvent event, const char* payload = nullptr);
    void process_loop();
    const char* get_state_name(CheckinState state) const;

    CheckinState get_state() const { return state_; }

private:
    CheckinState state_;
    QueueHandle_t event_queue;
}
};