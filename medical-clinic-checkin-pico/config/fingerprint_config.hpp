#ifndef FINGERPRINT_CONFIG_HPP
#define FINGERPRINT_CONFIG_HPP

#include "pin_config.hpp" // Inclui para pegar FP_UART_ID

/**
 * @file fingerprint_config.hpp
 * @brief Fingerprint sensor configuration for the Medical Clinic Check-in System
 * 
 * This file contains R307S fingerprint sensor hardware and driver settings.
 */

// ============================================================================
// UART Configuration
// ============================================================================

// Usa a definição centralizada
#define FP_UART_INSTANCE    FP_UART_ID 

// UART communication parameters
#define FP_BAUDRATE         57600  // R307S default: 57600 (9600*6)
#define FP_DATA_BITS        8      // Data bits (always 8 for R307S)
#define FP_STOP_BITS        1      // Stop bits (always 1 for R307S)
#define FP_PARITY           0      // Parity: 0=none (R307S uses no parity)

#endif // FINGERPRINT_CONFIG_HPP
