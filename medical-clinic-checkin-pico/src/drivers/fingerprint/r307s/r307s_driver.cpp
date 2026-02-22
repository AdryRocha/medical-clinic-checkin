#include "drivers/fingerprint/r307s/r307s_driver.hpp"
#include "services/logger_service.hpp" // <--- ADICIONADO: Corrige erro 'Logger not declared'

R307S_Driver::R307S_Driver(HAL_UART_Interface* uart)
    : uart_(uart) {}

void R307S_Driver::init() {
    Logger::info("[R307S] init");

    // Lógica original corrigida: Limpa o RX guardando no buffer (ou descartando)
    if (uart_) {
        uint8_t b;
        // CORREÇÃO: O loop while estava com as chaves erradas no seu arquivo
        while (uart_->available() > 0) { 
            int n = uart_->read(&b, 1);
            if (n == 1) {
                // Se você quiser guardar o lixo inicial:
                buffer_.push_back(b);
            } else {
                break;
            }
        }
    }
}

// Adicionei este método pois ele é exigido pela classe, 
// mas mantive simples para usar a sua lógica de buffer_
void R307S_Driver::poll() {
    if (!uart_) return;
    
    // Lê tudo que chegar e joga no buffer
    while (uart_->available() > 0) {
        uint8_t b;
        if (uart_->read(&b, 1) == 1) {
            buffer_.push_back(b);
        }
    }
}

bool R307S_Driver::available() const {
    return !buffer_.empty();
}

bool R307S_Driver::detectPacketStart(const std::vector<uint8_t>& data) const {
    // Header típico: 0xEF 0x01
    if (data.size() < 2) return false;
    return data[0] == 0xEF && data[1] == 0x01;
}

uint16_t R307S_Driver::calcChecksum(const std::vector<uint8_t>& d) const {
    uint16_t sum = 0;
    for (auto b : d) sum += b;
    return sum;
}

bool R307S_Driver::readPacket(std::vector<uint8_t>& pkt) {
    // Sua lógica original:
    if (buffer_.empty()) return false;
    
    // Verifica se tem header
    if (!detectPacketStart(buffer_)) {
        // Se o começo não é header, descarta o primeiro byte e tenta de novo depois
        // (Isso evita travar se chegar lixo antes do pacote)
        buffer_.erase(buffer_.begin());
        return false;
    }

    // Se chegou aqui, parece um pacote válido.
    // Copia para pkt e limpa buffer
    pkt = buffer_;
    buffer_.clear();
    return true;
}

bool R307S_Driver::isMatchPacket(const std::vector<uint8_t>& pkt) const {
    // Placeholder que você já tinha
    return pkt.size() > 9;
}

// Stubs necessários para o Linker não reclamar se estiverem no .hpp
bool R307S_Driver::handshake() { return false; }
bool R307S_Driver::verifyPassword(uint32_t) { return false; }