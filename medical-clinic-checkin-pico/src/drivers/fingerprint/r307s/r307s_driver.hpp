#pragma once

#include <cstdint>
#include <vector>
#include "hal/interfaces/hal_uart_interface.hpp"

class R307S_Driver {
public:
    explicit R307S_Driver(HAL_UART_Interface* uart);

    void init();
    void poll();
    bool available() const;

    // Adicionados para casar com o .cpp
    bool handshake();
    bool verifyPassword(uint32_t password);

private:
    bool detectPacketStart(const std::vector<uint8_t>& data) const;
    uint16_t calcChecksum(const std::vector<uint8_t>& d) const;
    bool readPacket(std::vector<uint8_t>& pkt);
    bool isMatchPacket(const std::vector<uint8_t>& pkt) const;

private:
    HAL_UART_Interface* uart_;
    std::vector<uint8_t> buffer_;
};