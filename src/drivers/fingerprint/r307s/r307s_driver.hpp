#ifndef R307S_DRIVER_HPP
#define R307S_DRIVER_HPP

#include "drivers/fingerprint/interface/fingerprint_interface.hpp"
#include "hal/interfaces/hal_uart_interface.hpp"
#include <vector>

/**
 * @brief R307S Fingerprint Sensor Driver
 * 
 * This driver implements the R307S fingerprint sensor protocol.
 * The R307S communicates via UART using a packet-based protocol.
 * 
 * Protocol Details:
 * - Baud rate: 57600 (default)
 * - Default password: 0x00000000
 * - Package format: Header(2) + Address(4) + PID(1) + Length(2) + Data(n) + Checksum(2)
 * 
 * Typical Usage:
 * 1. Enroll: getImage() -> image2Tz(1) -> getImage() -> image2Tz(2) -> createModel() -> storeModel()
 * 2. Match: getImage() -> image2Tz(1) -> fingerFastSearch()
 */
class R307S_Driver : public FingerprintInterface {
public:
    /**
     * @brief Construct an R307S driver
     * @param uart_hal Pre-initialized UART hardware abstraction layer
     * @param password Device password (default: 0x00000000)
     * @param address Device address (default: 0xFFFFFFFF)
     */
    explicit R307S_Driver(HAL_UART_Interface* uart_hal, 
                         uint32_t password = 0x00000000,
                         uint32_t address = 0xFFFFFFFF);

    // FingerprintInterface implementation
    bool init() override;
    bool verifyPassword() override;
    FingerprintStatus getImage() override;
    FingerprintStatus image2Tz(uint8_t slot) override;
    FingerprintStatus createModel() override;
    FingerprintStatus storeModel(uint16_t id) override;
    FingerprintStatus fingerFastSearch(FingerprintMatch& match) override;
    FingerprintStatus deleteModel(uint16_t id) override;
    FingerprintStatus emptyDatabase() override;
    FingerprintStatus getTemplateCount(uint16_t& count) override;
    FingerprintStatus enrollFingerprint(uint16_t id) override;
    FingerprintStatus matchFingerprint(FingerprintMatch& match) override;
    bool setLED(bool on) override;

    // Template transfer functions
    FingerprintStatus loadTemplate(uint16_t id, uint8_t slot) override;
    FingerprintStatus uploadTemplate(uint8_t slot, const std::vector<uint8_t>& data) override;
    FingerprintStatus downloadTemplate(uint8_t slot, std::vector<uint8_t>& data) override;
    FingerprintStatus compareTemplates(uint16_t& confidence) override;

    /**
     * @brief Get system parameters from the sensor
     * @param status_register Output parameter for status register
     * @param system_id Output parameter for system ID
     * @param library_size Output parameter for fingerprint library size
     * @param security_level Output parameter for security level
     * @return Status code
     */
    FingerprintStatus readSysPara(uint16_t& status_register, uint16_t& system_id, 
                                  uint16_t& library_size, uint16_t& security_level);

private:
    HAL_UART_Interface* uart_hal_;
    uint32_t password_;
    uint32_t address_;
    bool is_initialized_;
    size_t data_packet_size_;

    // Protocol constants
    static constexpr uint16_t START_CODE = 0xEF01;
    static constexpr uint8_t PID_COMMAND = 0x01;      // Command packet
    static constexpr uint8_t PID_DATA = 0x02;         // Data packet
    static constexpr uint8_t PID_ACK = 0x07;          // Acknowledge packet
    static constexpr uint8_t PID_END_DATA = 0x08;     // End of data packet

    // Command codes
    static constexpr uint8_t CMD_GEN_IMAGE = 0x01;       // Generate image
    static constexpr uint8_t CMD_IMAGE_2_TZ = 0x02;      // Generate character file from image
    static constexpr uint8_t CMD_MATCH = 0x03;           // Precise match two templates
    static constexpr uint8_t CMD_SEARCH = 0x04;          // Search fingerprint library
    static constexpr uint8_t CMD_REG_MODEL = 0x05;       // Combine character files
    static constexpr uint8_t CMD_STORE = 0x06;           // Store template
    static constexpr uint8_t CMD_LOAD = 0x07;            // Read template from flash
    static constexpr uint8_t CMD_UPLOAD_CHAR = 0x08;     // Upload template
    static constexpr uint8_t CMD_DOWNLOAD_CHAR = 0x09;   // Download template
    static constexpr uint8_t CMD_UPLOAD_IMAGE = 0x0A;    // Upload image
    static constexpr uint8_t CMD_DOWNLOAD_IMAGE = 0x0B;  // Download image
    static constexpr uint8_t CMD_DELETE = 0x0C;          // Delete templates
    static constexpr uint8_t CMD_EMPTY = 0x0D;           // Empty library
    static constexpr uint8_t CMD_SET_PASSWORD = 0x12;    // Set password
    static constexpr uint8_t CMD_VERIFY_PASSWORD = 0x13; // Verify password
    static constexpr uint8_t CMD_GET_RANDOM = 0x14;      // Get random number
    static constexpr uint8_t CMD_TEMPLATE_NUM = 0x1D;    // Get template count
    static constexpr uint8_t CMD_READ_SYS_PARA = 0x0F;   // Read system parameters
    static constexpr uint8_t CMD_LED_CONFIG = 0x35;      // LED control
    static constexpr uint8_t CMD_AURA_LED_CONFIG = 0x35; // Aura LED control

    // Buffer sizes
    static constexpr size_t PACKET_BUFFER_SIZE = 256;
    static constexpr uint32_t DEFAULT_TIMEOUT_MS = 1000;
    static constexpr uint32_t IMAGE_TIMEOUT_MS = 5000;   // Longer timeout for image capture

    uint8_t packet_buffer_[PACKET_BUFFER_SIZE];

    /**
     * @brief Send a command packet to the sensor
     * @param command Command code
     * @param data Data bytes to send (can be nullptr)
     * @param data_len Length of data
     * @return true if packet sent successfully
     */
    bool sendPacket(uint8_t command, const uint8_t* data = nullptr, size_t data_len = 0);

    /**
     * @brief Receive a response packet from the sensor
     * @param timeout_ms Timeout in milliseconds
     * @return Number of bytes received (0 on error)
     */
    size_t receivePacket(uint32_t timeout_ms = DEFAULT_TIMEOUT_MS);

    /**
     * @brief Receive data packets (for large transfers like templates)
     * @param data Output vector to store received data
     * @param timeout_ms Timeout in milliseconds
     * @return Number of bytes received (0 on error)
     */
    size_t receiveDataPacket(std::vector<uint8_t>& data, uint32_t timeout_ms = DEFAULT_TIMEOUT_MS);

    /**
     * @brief Send data packets (for large transfers like templates)
     * @param data Data to send
     * @return true if all packets sent successfully
     */
    bool sendDataPacket(const std::vector<uint8_t>& data);

    /**
     * @brief Get the confirmation code from the last received packet
     * @return Confirmation code (status)
     */
    uint8_t getConfirmationCode();

    /**
     * @brief Calculate checksum for packet
     * @param buffer Buffer to calculate checksum for
     * @param len Length of buffer
     * @return Checksum value
     */
    uint16_t calculateChecksum(const uint8_t* buffer, size_t len);

    /**
     * @brief Write 16-bit value to buffer (big-endian)
     */
    void writeU16(uint8_t* buffer, uint16_t value);

    /**
     * @brief Write 32-bit value to buffer (big-endian)
     */
    void writeU32(uint8_t* buffer, uint32_t value);

    /**
     * @brief Read 16-bit value from buffer (big-endian)
     */
    uint16_t readU16(const uint8_t* buffer);

    /**
     * @brief Read 32-bit value from buffer (big-endian)
     */
    uint32_t readU32(const uint8_t* buffer);
};

#endif // R307S_DRIVER_HPP
