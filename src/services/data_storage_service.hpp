#ifndef DATA_STORAGE_SERVICE_HPP
#define DATA_STORAGE_SERVICE_HPP

#include <string>
#include <vector>
#include "f_util.h"
#include "ff.h"

/**
 * @brief Service for managing appointment data storage on SD card
 */
class DataStorageService {
public:
    static DataStorageService& getInstance();
    
    /**
     * @brief Initialize SD card and create directories
     * @return true if successful
     */
    bool init();
    
    /**
     * @brief Save appointments JSON data for a specific date
     * @param date Date in format "YYYY-MM-DD"
     * @param json_data JSON string containing appointments
     * @return true if saved successfully
     */
    bool saveAppointments(const std::string& date, const std::string& json_data);
    
    /**
     * @brief Check if appointments file exists for a date
     * @param date Date in format "YYYY-MM-DD"
     * @return true if file exists
     */
    bool hasAppointments(const std::string& date);
    
    /**
     * @brief Read appointments JSON data for a specific date
     * @param date Date in format "YYYY-MM-DD"
     * @param json_data Output parameter for JSON string
     * @return true if read successfully
     */
    bool readAppointments(const std::string& date, std::string& json_data);
    
    /**
     * @brief Get tomorrow's date string in YYYY-MM-DD format
     * @return Date string or empty if RTC not synced
     */
    std::string getTomorrowDate();
    
    /**
     * @brief Get today's date string in YYYY-MM-DD format
     * @return Date string or empty if RTC not synced
     */
    std::string getTodayDate();
    
    /**
     * @brief Check if SD card is mounted and ready
     * @return true if ready
     */
    bool isReady();
    
    /**
     * @brief Mark appointment as completed in local JSON
     * @param date Date in format "YYYY-MM-DD"
     * @param appointment_id ID of the appointment to mark
     * @return true if updated successfully
     */
    bool markAppointmentCompleted(const std::string& date, int appointment_id);
    
    /**
     * @brief Check if appointment is already completed
     * @param date Date in format "YYYY-MM-DD"
     * @param appointment_id ID to check
     * @return true if status is "realizada"
     */
    bool isAppointmentCompleted(const std::string& date, int appointment_id);
    
    /**
     * @brief Save fingerprint template to SD card
     * @param patient_id Patient ID
     * @param template_data Template binary data
     * @return true if saved successfully
     */
    bool saveFingerprintTemplate(int patient_id, const std::vector<uint8_t>& template_data);
    
    /**
     * @brief Load fingerprint template from SD card
     * @param patient_id Patient ID
     * @param template_data Output parameter for template data
     * @return true if loaded successfully
     */
    bool loadFingerprintTemplate(int patient_id, std::vector<uint8_t>& template_data);
    
    /**
     * @brief Check if fingerprint template exists
     * @param patient_id Patient ID
     * @return true if template file exists
     */
    bool hasFingerprintTemplate(int patient_id);
    
    /**
     * @brief Delete fingerprint template
     * @param patient_id Patient ID
     * @return true if deleted successfully
     */
    bool deleteFingerprintTemplate(int patient_id);
    
    /**
     * @brief Clear all fingerprint templates
     * @return true if successful
     */
    bool clearAllFingerprints();
    
private:
    DataStorageService();
    ~DataStorageService();
    DataStorageService(const DataStorageService&) = delete;
    DataStorageService& operator=(const DataStorageService&) = delete;
    
    bool mounted_;
    FATFS fatfs_;
    
    std::string getFilePath(const std::string& date);
};

#endif // DATA_STORAGE_SERVICE_HPP