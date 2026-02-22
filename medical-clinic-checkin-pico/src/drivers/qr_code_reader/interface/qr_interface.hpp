#pragma once
#ifndef QR_INTERFACE_HPP
#define QR_INTERFACE_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

class QR_Interface {
public:
    using ScanCallback = std::function<void(const std::string& data)>;

    virtual ~QR_Interface() = default;

    // Inicializa o leitor (UART/config etc.)
    virtual bool init() = 0;

    // Loop/poll do driver (chame periodicamente na task)
    virtual void process() = 0;

    // Indica se há um scan pronto para leitura
    virtual bool isScanAvailable() = 0;

    // Lê o scan para um buffer do usuário (retorna bytes copiados)
    virtual size_t readScan(char* buffer, size_t max_length, uint32_t timeout_ms) = 0;

    // Callback opcional quando um scan é recebido
    virtual void setScanCallback(ScanCallback callback) = 0;

    virtual bool enableScan(bool enable) = 0;

    virtual bool triggerScan() = 0;
};

#endif // QR_INTERFACE_HPP
