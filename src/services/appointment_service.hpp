#ifndef APPOINTMENT_SERVICE_HPP
#define APPOINTMENT_SERVICE_HPP

#include <string>
#include <cstdint>

// Forward declaration
struct cJSON;

/**
 * @brief Appointment information structure
 */
struct AppointmentInfo {
    int id;
    std::string patient_name;
    std::string patient_cpf;
    std::string professional_name;
    std::string category;
    std::string time;
    std::string status;
    std::string qr_hash;  // Hash from QR code for validation
    bool found;
    
    // Biometry information
    struct PatientBiometry {
        int id;                    // patient_id
        bool aceita_digital;
        bool fingerprint_uploaded;
    } patient;
    
    bool requires_fingerprint_verification;
    bool requires_fingerprint_enrollment;
    
    AppointmentInfo() : 
        id(0), 
        found(false),
        requires_fingerprint_verification(false),
        requires_fingerprint_enrollment(false) {
        patient.id = 0;
        patient.aceita_digital = false;
        patient.fingerprint_uploaded = false;
    }
};

/**
 * @brief QR Code data structure
 */
struct QRCodeData {
    std::string cmd;
    int appt_id;
    std::string cpf;
    std::string name;
    std::string hash;
    bool valid;
    
    QRCodeData() : appt_id(0), valid(false) {}
};

/**
 * @brief Service for validating and managing appointments
 */
class AppointmentService {
public:
    static AppointmentService& getInstance();
    
    /**
     * @brief Validate and parse QR code JSON data
     * @param qr_json QR code JSON string
     * @param qr_data Output parameter with parsed QR data
     * @return true if QR code is valid and parsed successfully
     */
    bool validateAndParseQRCode(const std::string& qr_json, QRCodeData& qr_data);
    
    /**
     * @brief Mask CPF showing only first 4 digits
     * @param cpf CPF string to mask
     * @return Masked CPF (e.g., "9876*******")
     */
    static std::string maskCPF(const std::string& cpf);
    
    /**
     * @brief Validate appointment by ID from QR code data
     * @param qr_data Parsed QR code data containing appointment ID and CPF
     * @param info Output parameter with appointment information
     * @return true if appointment found, valid, and within time window
     */
    bool validateAppointmentById(const QRCodeData& qr_data, AppointmentInfo& info);
    
    /**
     * @brief Update appointment status to "realizada" (completed)
     * @param appointment_id The appointment ID to update
     * @return true if updated successfully (local and API)
     */
    bool markAppointmentCompleted(int appointment_id);
    
private:
    AppointmentService();
    
    // Helper methods for validation
    bool findAppointmentById(cJSON* root, int appt_id, cJSON** appointment_out);
    void extractAppointmentData(cJSON* appointment, AppointmentInfo& info);
    bool validateCPFMatch(const std::string& qr_cpf, const std::string& db_cpf);
    bool validateAppointmentTime(const std::string& appointment_time);
    ~AppointmentService();
    AppointmentService(const AppointmentService&) = delete;
    AppointmentService& operator=(const AppointmentService&) = delete;
    
    /**
     * @brief Clean CPF string (remove dots, dashes, spaces)
     * @param cpf CPF string to clean
     * @return Cleaned CPF with only numbers
     */
    std::string cleanCPF(const std::string& cpf);
};

#endif // APPOINTMENT_SERVICE_HPP