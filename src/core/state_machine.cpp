#include "core/state_machine.hpp"

StateMachine::StateMachine() : state_(State::INITIALIZING) {
}

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