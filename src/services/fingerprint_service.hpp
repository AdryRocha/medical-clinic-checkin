#ifndef FINGERPRINT_SERVICE_HPP
#define FINGERPRINT_SERVICE_HPP

#include <vector>
#include <cstdint>

// Forward declarations
class R307S_Driver;
class HAL_UART_RP2040;

/**
 * @brief Service for managing fingerprint sensor operations
 * 
 * This service handles:
 * - Sensor initialization
 * - Fingerprint enrollment (capturing and storing templates)
 * - Fingerprint verification against stored templates
 * - Integration with SD card storage via DataStorageService
 */
class FingerprintService {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to the fingerprint service instance
     */
    static FingerprintService& getInstance();
    
    /**
     * @brief Initialize the fingerprint sensor
     * @return true if initialization successful
     */
    bool init();
    
    /**
     * @brief Check if sensor is initialized
     * @return true if ready for operations
     */
    bool isInitialized() const { return is_initialized_; }
    
    /**
     * @brief Enroll a new fingerprint (captures 2 scans and creates template)
     * 
     * This method guides the user through:
     * 1. Place finger
     * 2. Remove finger
     * 3. Place same finger again
     * 4. Generate and return template
     * 
     * @param template_data Output parameter - will contain the fingerprint template
     * @param timeout_per_scan_ms Timeout for each scan (default: 15 seconds)
     * @return true if enrollment successful and template captured
     */
    bool enrollFingerprint(std::vector<uint8_t>& template_data,
                          int timeout_per_scan_ms = 15000);
    
    /**
     * @brief Verify a fingerprint against a patient's stored template
     * 
     * This method:
     * 1. Loads the patient's template from SD card
     * 2. Uploads it to sensor slot 1
     * 3. Captures current finger
     * 4. Compares against stored template
     * 
     * @param patient_id Patient ID to load template for
     * @param confidence Output parameter - confidence score (0-100+)
     * @param timeout_ms Timeout for finger scan (default: 15 seconds)
     * @return true if finger matches template
     */
    bool verifyFingerprint(int patient_id, 
                          uint16_t& confidence,
                          int timeout_ms = 15000);
    
    /**
     * @brief Clear all templates from sensor memory
     * @return true if successful
     */
    bool clearSensorMemory();
    
    /**
     * @brief Get template count in sensor
     * @param count Output parameter for template count
     * @return true if successful
     */
    bool getTemplateCount(uint16_t& count);

private:
    FingerprintService();
    ~FingerprintService();
    FingerprintService(const FingerprintService&) = delete;
    FingerprintService& operator=(const FingerprintService&) = delete;
    
    R307S_Driver* fp_driver_;
    HAL_UART_RP2040* uart_hal_;
    bool is_initialized_;
    
    /**
     * @brief Load a template from vector to sensor slot
     * @param data Template data
     * @param slot Slot number (1 or 2)
     * @return true if successful
     */
    bool loadTemplateToSensor(const std::vector<uint8_t>& data, uint8_t slot);
    
    /**
     * @brief Wait for finger to be placed on sensor
     * @param timeout_ms Timeout in milliseconds
     * @return true if finger detected
     */
    bool waitForFinger(int timeout_ms);
    
    /**
     * @brief Wait for finger to be removed from sensor
     * @param timeout_ms Timeout in milliseconds
     * @return true if finger removed
     */
    bool waitForFingerRemoval(int timeout_ms);
};

#endif // FINGERPRINT_SERVICE_HPP