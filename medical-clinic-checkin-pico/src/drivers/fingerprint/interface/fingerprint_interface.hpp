#ifndef FINGERPRINT_INTERFACE_HPP
#define FINGERPRINT_INTERFACE_HPP

#include <cstdint>
#include <cstddef>
#include <vector>

/**
 * @brief Fingerprint sensor status codes
 */
enum class FingerprintStatus {
    OK = 0x00,                      // Success
    ERROR_COMM = 0x01,              // Communication error
    ERROR_NO_FINGER = 0x02,         // No finger detected
    ERROR_ENROLL_FAIL = 0x03,       // Failed to enroll
    ERROR_BAD_IMAGE = 0x06,         // Failed to generate image
    ERROR_TOO_MESSY = 0x07,         // Image too messy
    ERROR_FEATURE_FAIL = 0x08,      // Failed to generate features
    ERROR_NO_MATCH = 0x09,          // No matching fingerprint
    ERROR_NOT_FOUND = 0x0A,         // ID not found
    ERROR_MERGE_FAIL = 0x0B,        // Failed to merge
    ERROR_BAD_LOCATION = 0x0B,      // Bad location/ID
    ERROR_DELETE_FAIL = 0x10,       // Failed to delete
    ERROR_CLEAR_FAIL = 0x11,        // Failed to clear database
    ERROR_WRONG_PASSWORD = 0x13,    // Wrong password
    ERROR_NO_TEMPLATE = 0x15,       // No template at location
    ERROR_UPLOAD_FAIL = 0x18,       // Failed to upload template
    ERROR_RECV_FAIL = 0x19,         // Failed to receive data
    ERROR_UPLOAD_IMAGE_FAIL = 0x1A, // Failed to upload image
    ERROR_DELETE_IMAGE_FAIL = 0x1B, // Failed to delete image
    ERROR_INVALID_REGISTER = 0x1A,  // Invalid register
    ERROR_TIMEOUT = 0xFF            // Custom timeout error
};

/**
 * @brief Fingerprint match result
 */
struct FingerprintMatch {
    uint16_t id;           // Matched fingerprint ID
    uint16_t confidence;   // Match confidence score (0-65535)
    bool matched;          // True if a match was found
};

/**
 * @brief Abstract interface for fingerprint sensors
 */
class FingerprintInterface {
public:
    virtual ~FingerprintInterface() = default;

    /**
     * @brief Initialize the fingerprint sensor
     * @return true if initialization successful
     */
    virtual bool init() = 0;

    /**
     * @brief Verify the password/connection with the sensor
     * @return true if verification successful
     */
    virtual bool verifyPassword() = 0;

    /**
     * @brief Get an image from the sensor
     * @return Status code
     */
    virtual FingerprintStatus getImage() = 0;

    /**
     * @brief Convert image to fingerprint template
     * @param slot Buffer slot (1 or 2)
     * @return Status code
     */
    virtual FingerprintStatus image2Tz(uint8_t slot) = 0;

    /**
     * @brief Create a model from templates in buffer slots 1 and 2
     * @return Status code
     */
    virtual FingerprintStatus createModel() = 0;

    /**
     * @brief Store the model in the specified ID location
     * @param id Location ID (1-65535)
     * @return Status code
     */
    virtual FingerprintStatus storeModel(uint16_t id) = 0;

    /**
     * @brief Search the fingerprint database for a match
     * @param match Output parameter for match result
     * @return Status code
     */
    virtual FingerprintStatus fingerFastSearch(FingerprintMatch& match) = 0;

    /**
     * @brief Delete a fingerprint from the database
     * @param id Fingerprint ID to delete
     * @return Status code
     */
    virtual FingerprintStatus deleteModel(uint16_t id) = 0;

    /**
     * @brief Clear the entire fingerprint database
     * @return Status code
     */
    virtual FingerprintStatus emptyDatabase() = 0;

    /**
     * @brief Get the number of stored templates
     * @param count Output parameter for template count
     * @return Status code
     */
    virtual FingerprintStatus getTemplateCount(uint16_t& count) = 0;

    /**
     * @brief Enroll a new fingerprint (simplified high-level function)
     * @param id Location to store the fingerprint
     * @return Status code (OK if successful)
     */
    virtual FingerprintStatus enrollFingerprint(uint16_t id) = 0;

    /**
     * @brief Match a fingerprint (simplified high-level function)
     * @param match Output parameter for match result
     * @return Status code (OK if successful)
     */
    virtual FingerprintStatus matchFingerprint(FingerprintMatch& match) = 0;

    /**
     * @brief Load a template from sensor's flash memory to buffer
     * @param id Template ID to load from flash
     * @param slot Buffer slot to load into (1 or 2)
     * @return Status code
     */
    virtual FingerprintStatus loadTemplate(uint16_t id, uint8_t slot) = 0;

    /**
     * @brief Upload template data from memory to sensor buffer
     * @param slot Buffer slot to upload to (1 or 2)
     * @param data Template data to upload
     * @return Status code
     */
    virtual FingerprintStatus uploadTemplate(uint8_t slot, const std::vector<uint8_t>& data) = 0;

    /**
     * @brief Download template data from sensor buffer to memory
     * @param slot Buffer slot to download from (1 or 2)
     * @param data Output parameter for template data
     * @return Status code
     */
    virtual FingerprintStatus downloadTemplate(uint8_t slot, std::vector<uint8_t>& data) = 0;

    /**
     * @brief Compare templates in buffer slots 1 and 2
     * @param confidence Output parameter for match confidence score (0-65535)
     * @return Status code (OK if templates match, ERROR_NO_MATCH if not)
     */
    virtual FingerprintStatus compareTemplates(uint16_t& confidence) = 0;

    /**
     * @brief LED control
     * @param on true to turn LED on, false for off
     * @return true if successful
     */
    virtual bool setLED(bool on) = 0;
};

#endif // FINGERPRINT_INTERFACE_HPP
