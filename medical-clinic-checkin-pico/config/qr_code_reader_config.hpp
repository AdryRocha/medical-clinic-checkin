#ifndef QR_CODE_READER_CONFIG_HPP
#define QR_CODE_READER_CONFIG_HPP

#include "pin_config.hpp" // Inclui para pegar QR_UART_ID

/**
 * @file qr_code_reader_config.hpp
 * @brief QR Scanner configuration
 */

// ============================================================================
// UART Configuration
// ============================================================================

// Usa a definição centralizada do pin_config.hpp
// Isso evita conflitos onde um arquivo diz uart0 e outro diz uart1

// Usa a definição centralizada do pin_config.hpp
#define QR_UART_INSTANCE    QR_UART_ID

// UART communication parameters
#define QR_BAUDRATE         9600    // GM67 padrão
#define QR_DATA_BITS        8      
#define QR_STOP_BITS        1      
#define QR_PARITY           0      

#endif // QR_CODE_READER_CONFIG_HPP