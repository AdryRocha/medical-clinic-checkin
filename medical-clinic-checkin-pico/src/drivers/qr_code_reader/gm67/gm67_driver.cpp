#include "gm67_driver.hpp"
#include <cstring> // Para memset, memcpy

// CORREÇÃO: Usando os nomes definidos no .hpp (scan_available_, last_scan_)
GM67_Driver::GM67_Driver(HAL_UART_Interface* uart)
    : uart_(uart), last_len_(0), scan_available_(false), callback_(nullptr) {
    memset(last_scan_, 0, sizeof(last_scan_));
}

bool GM67_Driver::init() {
    if (!uart_) return false;
    // O init da UART agora pede apenas baudrate
    return uart_->init(9600); 
}

void GM67_Driver::process() {
    if (!uart_) return;

    while (uart_->available() > 0) {
        // CORREÇÃO: Usando readByte() que retorna uint8_t diretamente
        // ou read(&byte, 1) se preferir verificar sucesso.
        // Assumindo que sua interface tem readByte() que retorna 0 se falhar ou o byte lido.
        
        uint8_t byte = uart_->readByte(); 
        
        // Se sua interface UART retornar void no readByte ou algo diferente, 
        // use: uart_->read(&byte, 1);
        
        // Lógica de fim de linha (CR ou LF)
        if (byte == '\r' || byte == '\n') {
            if (last_len_ > 0) {
                last_scan_[last_len_] = '\0';
                scan_available_ = true;
                
                if (callback_) callback_(last_scan_);
            }
        } 
        else {
            // Se chegou caractere novo e já tinha scan pronto, limpa para começar outro
            if (scan_available_) {
                clearLast();
            }

            if (last_len_ < sizeof(last_scan_) - 1) {
                last_scan_[last_len_++] = (char)byte;
            }
        }
    }
}

// CORREÇÃO: Removido 'const' para bater com o header
bool GM67_Driver::isScanAvailable() {
    return scan_available_;
}

size_t GM67_Driver::readScan(char* buffer, size_t max_length, uint32_t /*timeout_ms*/) {
    if (!scan_available_ || last_len_ == 0) return 0;

    size_t to_copy = (last_len_ < max_length) ? last_len_ : (max_length - 1);
    
    memcpy(buffer, last_scan_, to_copy);
    buffer[to_copy] = '\0';

    // clearLast(); // Opcional: limpar após leitura
    return to_copy;
}

void GM67_Driver::setScanCallback(ScanCallback callback) {
    callback_ = callback;
}

void GM67_Driver::clearLast() {
    last_len_ = 0;
    scan_available_ = false;
    memset(last_scan_, 0, sizeof(last_scan_));
}