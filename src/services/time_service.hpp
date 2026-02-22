#ifndef TIME_SERVICE_HPP
#define TIME_SERVICE_HPP

#include <cstdint>
#include <string>
#include <ctime>

/**
 * @brief Simple time service for NTP synchronization and time display
 */
class TimeService {
public:
    static TimeService& getInstance();
    
    /**
     * @brief Initialize the time service
     * @return true if successful
     */
    bool init();
    
    /**
     * @brief Get current time as formatted string
     * @return Time string in format "HH:MM:SS" or "00:00:00" if not synced
     */
    std::string getTimeString();
    
    /**
     * @brief Get current date as formatted string
     * @return Date string in format "DD/MM/YYYY" or "00/00/0000" if not synced
     */
    std::string getDateString();
    
    /**
     * @brief Get current date and time as formatted string
     * @return DateTime string in format "DD/MM/YYYY HH:MM:SS"
     */
    std::string getDateTimeString();
    
    /**
     * @brief Check if time has been synchronized
     * @return true if time is valid
     */
    bool isTimeSynced();
    
    /**
     * @brief Mark time as synchronized (called from SNTP callback)
     */
    void markTimeSynced();
    
    /**
     * @brief Extract hour and minute from time string
     * @param time_str Time string in format "HH:MM" or "HH:MM:SS"
     * @param hour Output parameter for hour (0-23)
     * @param min Output parameter for minute (0-59)
     * @return true if successfully extracted
     */
    static bool extractHourMinute(const std::string& time_str, int& hour, int& min);
    
private:
    TimeService();
    ~TimeService();
    TimeService(const TimeService&) = delete;
    TimeService& operator=(const TimeService&) = delete;
    
    bool initialized_;
    bool time_synced_;
};

#endif // TIME_SERVICE_HPP