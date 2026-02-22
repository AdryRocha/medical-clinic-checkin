#ifndef QR_CODE_READER_CONFIG_HPP
#define QR_CODE_READER_CONFIG_HPP

/**
 * @file qr_code_reader_config.hpp
 * @brief QR Scanner configuration for the Medical Clinic Check-in System
 * 
 * This file contains QR scanner (GM67) hardware and driver settings.
 * Adjust these values based on your GM67 model and requirements.
 */

// ============================================================================
// UART Configuration
// ============================================================================

// UART instance selection
// Production: uart0 (GP0/GP1 for QR), uart1 (GP4/GP5 for Fingerprint)
#define QR_UART_INSTANCE    uart0  // uart0 for GPIO0/1, uart1 for GPIO4/5

// UART communication parameters
#define QR_BAUDRATE         115200 // GM67 default is often 115200, not 9600!
#define QR_DATA_BITS        8      // Data bits (typically 8)
#define QR_STOP_BITS        1      // Stop bits (1 or 2)
#define QR_PARITY           0      // Parity: 0=none, 1=odd, 2=even

#endif // QR_CODE_READER_CONFIG_HPP