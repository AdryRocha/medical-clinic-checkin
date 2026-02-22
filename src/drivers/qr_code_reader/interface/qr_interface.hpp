#ifndef QR_INTERFACE_HPP
#define QR_INTERFACE_HPP

#include <string>
#include <functional>
#include <stdint.h>
#include <stddef.h>

// Removido: #include "hal_uart_interface.hpp" 
// A interface não precisa saber sobre UART. Quem precisa é o Driver (GM67).

/**
 * @brief Generic QR/Barcode Scanner Interface
 * * This interface defines the contract for QR scanner drivers.
 * Different scanner models can implement this interface.
 */
class QR_Interface {
public:
    /**
     * @brief Callback function type for scanned data
     * @param data The scanned QR/barcode data
     */
    using ScanCallback = std::function<void(const std::string& data)>;

    virtual ~QR_Interface() = default;

    /**
     * @brief Initialize the QR scanner
     * @return true if initialization successful
     */
    virtual bool init() = 0;

    /**
     * @brief Read scanned data from the scanner
     * @param buffer Buffer to store the scanned data
     * @param max_length Maximum buffer length
     * @param timeout_ms Timeout in milliseconds (0 = no timeout)
     * @return Number of bytes read, or 0 if no data/timeout
     */
    virtual size_t readScan(char* buffer, size_t max_length, uint32_t timeout_ms = 0) = 0;

    /**
     * @brief Check if scan data is available
     * @return true if data is available
     */
    virtual bool isScanAvailable() = 0;

    /**
     * @brief Set scan callback function
     * @param callback Function to call when data is scanned
     */
    virtual void setScanCallback(ScanCallback callback) = 0;

    /**
     * @brief Process incoming data (call this regularly in main loop or task)
     */
    virtual void process() = 0;

    /**
     * @brief Enable/disable scanner
     * @param enable true to enable scanning, false to disable
     * @return true if command successful
     */
    virtual bool enableScan(bool enable) = 0;

    /**
     * @brief Trigger a single scan
     * @return true if command successful
     */
    virtual bool triggerScan() = 0;
};

#endif // QR_INTERFACE_HPP