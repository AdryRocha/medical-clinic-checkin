#ifndef ERROR_CRITICAL_SCREEN_HPP
#define ERROR_CRITICAL_SCREEN_HPP

/**
 * @brief Show critical error screen
 */
void error_critical_screen_show();

/**
 * @brief Update error message and countdown
 * @param title Error title
 * @param message Error message
 * @param countdown_seconds Number of seconds until reboot
 */
void error_critical_screen_update(const char* title, const char* message, int countdown_seconds);

/**
 * @brief Update only the countdown on critical error screen
 * @param seconds_remaining Seconds remaining until reboot
 */
void error_critical_screen_update_countdown(int seconds_remaining);

/**
 * @brief Clear screen before reboot to avoid flicker
 */
void error_critical_screen_clear();

#endif