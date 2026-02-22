#pragma once
#ifndef GM67_DRIVER_HPP
#define GM67_DRIVER_HPP

#include <cstddef>
#include <cstdint>

#include "drivers/qr_code_reader/interface/qr_interface.hpp"
#include "hal/interfaces/hal_uart_interface.hpp"

class GM67_Driver : public QR_Interface {
public:
    explicit GM67_Driver(HAL_UART_Interface* uart);

    bool init() override;
    void process() override;

    bool isScanAvailable() override;
    size_t readScan(char* buffer, size_t max_length, uint32_t timeout_ms) override;

    void setScanCallback(ScanCallback callback) override;

private:
    HAL_UART_Interface* uart_ = nullptr;

    ScanCallback callback_ = nullptr;

    static constexpr size_t kMaxScanLen = 256;
    char last_scan_[kMaxScanLen]{};
    size_t last_len_ = 0;
    bool scan_available_ = false;

    void clearLast();
};

#endif // GM67_DRIVER_HPP
