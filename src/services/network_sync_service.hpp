#ifndef NETWORK_SYNC_SERVICE_HPP
#define NETWORK_SYNC_SERVICE_HPP

#include <string>
#include <vector>
#include <cstdint>
#include "mongoose.h"

/**
 * @brief Service for syncing appointments from REST API
 */
class NetworkSyncService {
public:
    static NetworkSyncService& getInstance();
    
    /**
     * @brief Authenticate with API and get access token
     * @param timeout_ms Timeout in milliseconds
     * @return true if authentication successful
     */
    bool authenticate(uint32_t timeout_ms = 10000);
    
    /**
     * @brief Download appointments for a specific date
     * @param date Date in format "YYYY-MM-DD"
     * @param json_response Output parameter for JSON response
     * @param timeout_ms Timeout in milliseconds
     * @return true if download successful
     */
    bool downloadAppointments(const std::string& date, 
                             std::string& json_response,
                             uint32_t timeout_ms = 30000);
    
    /**
     * @brief Check if we have a valid token
     * @return true if token is available
     */
    bool hasToken() const { return !access_token_.empty(); }
    
    /**
     * @brief Update appointment status in API
     * @param appointment_id ID of the appointment
     * @param new_status New status (e.g., "realizada")
     * @param timeout_ms Timeout in milliseconds
     * @return true if update successful
     */
    bool updateAppointmentStatus(int appointment_id, 
                                  const char* new_status,
                                  uint32_t timeout_ms = 10000);
    
    /**
     * @brief Download fingerprint template from API
     * @param patient_id Patient ID
     * @param template_data Output parameter for template binary data
     * @param timeout_ms Timeout in milliseconds
     * @return true if download successful
     */
    bool downloadFingerprintTemplate(int patient_id,
                                      std::vector<uint8_t>& template_data,
                                      uint32_t timeout_ms = 10000);
    
    /**
     * @brief Upload fingerprint template to API
     * @param patient_id Patient ID
     * @param template_data Template binary data
     * @param timeout_ms Timeout in milliseconds
     * @return true if upload successful
     */
    bool uploadFingerprintTemplate(int patient_id,
                                    const std::vector<uint8_t>& template_data,
                                    uint32_t timeout_ms = 15000);
    
    /**
     * @brief Download all fingerprints for appointments (batch)
     * @param json_appointments JSON string containing appointments
     * @return Number of fingerprints successfully downloaded
     */
    int downloadFingerprintsForAppointments(const std::string& json_appointments);
    
private:
    NetworkSyncService();
    ~NetworkSyncService();
    NetworkSyncService(const NetworkSyncService&) = delete;
    NetworkSyncService& operator=(const NetworkSyncService&) = delete;
    
    struct RequestContext {
        std::string* response;
        bool completed;
        bool success;
    };
    
    static void httpCallback(struct mg_connection *c, int ev, void *ev_data);
    
    std::string access_token_;
};

#endif // NETWORK_SYNC_SERVICE_HPP