#ifndef GM67_DRIVER_HPP
#define GM67_DRIVER_HPP

#include "drivers/qr_code_reader/interface/qr_interface.hpp"
#include "hal/interfaces/hal_uart_interface.hpp"

/**
 * @brief GM67 QR/Barcode Scanner Driver
 * 
 * This driver supports the GM67 series QR code scanner modules.
 * The GM67 communicates via UART and sends scanned data as ASCII strings.
 */
class GM67_Driver : public QR_Interface {
public:
    /**
     * @brief Construct a GM67 driver
     * @param uart_hal Pre-initialized UART hardware abstraction layer
     * @note The UART HAL must be initialized before passing to this driver
     */
    explicit GM67_Driver(HAL_UART_Interface* uart_hal);

    bool init() override;
    size_t readScan(char* buffer, size_t max_length, uint32_t timeout_ms = 0) override;
    bool isScanAvailable() override;
    void setScanCallback(ScanCallback callback) override;
    void process() override;
    bool enableScan(bool enable) override;
    bool triggerScan() override;

    /**
     * @brief Send command to scanner
     * @param command Command string
     * @return true if command sent successfully
     */
    bool sendCommand(const char* command);

    /**
     * @brief Set scanner to continuous scan mode
     * @return true if command successful
     */
    bool setContinuousMode();

    /**
     * @brief Set scanner to command scan mode
     * @return true if command successful
     */
    bool setCommandMode();

private:
    HAL_UART_Interface* uart_hal_;
    ScanCallback scan_callback_;
    
    static constexpr size_t BUFFER_SIZE = 1024;
    char rx_buffer_[BUFFER_SIZE];
    size_t rx_index_;
    uint32_t last_scan_time_;
    
    bool is_initialized_;
    
    /**
     * @brief Process received character
     * @param ch Character received
     */
    void processChar(char ch);
    
    /**
     * @brief Handle complete scan data
     */
    void handleScanComplete();
};

#endif // GM67_DRIVER_HPP