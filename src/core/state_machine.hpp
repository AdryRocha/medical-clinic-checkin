#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

/**
 * @brief State Machine
 * Manages application states for the check-in system
 */
class StateMachine {
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

    /**
     * @brief Get singleton instance
     * @return Reference to the state machine instance
     */
    static StateMachine& getInstance();

    /**
     * @brief Set current state
     * @param newState The new state to transition to
     */
    void setState(State newState);

    /**
     * @brief Get current state
     * @return Current state
     */
    State getState() const;

    /**
     * @brief Get current state name as string
     * @return State name for debugging
     */
    const char* getStateName() const;

    /**
     * @brief Get state name for given state
     * @param state State to get name for
     * @return State name string
     */
    const char* getStateName(State state) const;

private:
    StateMachine();
    StateMachine(const StateMachine&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;

    volatile State state_;
};

#endif // STATE_MACHINE_HPP