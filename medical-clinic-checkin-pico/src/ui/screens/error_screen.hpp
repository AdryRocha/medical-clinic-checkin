#ifndef ERROR_SCREEN_HPP
#define ERROR_SCREEN_HPP

/**
 * @brief Show error screen
 * Uses stored message from error_screen_update()
 */
void error_screen_show();

/**
 * @brief Update error screen content before showing
 * Call this BEFORE setting state to ERROR
 * @param title Error title (ex: "Consulta não encontrada", "QR Code inválido")
 * @param message Friendly explanation (ex: "Não encontramos seu cadastro no sistema")
 * @param detail Additional info (optional, ex: patient name + CPF)
 */
void error_screen_update(const char* title, 
                         const char* message,
                         const char* detail = nullptr);

#endif // ERROR_SCREEN_HPP