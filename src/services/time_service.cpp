#include "time_service.hpp"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include "hardware/rtc.h"
#include "lwip/apps/sntp.h"
#include <cstdio>
#include <ctime>

// #define DEBUG_FAKE_TIME

//#ifdef DEBUG_FAKE_TIME
//    #define FAKE_YEAR   2026
//    #define FAKE_MONTH  2
//    #define FAKE_DAY    2
//    #define FAKE_HOUR   8
//    #define FAKE_MINUTE 10
//    #warning "DEBUG_FAKE_TIME ENABLED - Using fake time for testing!"
//#endif


TimeService& TimeService::getInstance() {
    static TimeService instance;
    return instance;
}

TimeService::TimeService() 
    : initialized_(false), time_synced_(false) {
}

TimeService::~TimeService() {
}

extern "C" void sntp_set_system_time(uint32_t sec, uint32_t us) {
#ifdef DEBUG_FAKE_TIME
    datetime_t dt;
    dt.year = FAKE_YEAR;
    dt.month = FAKE_MONTH;
    dt.day = FAKE_DAY;
    dt.hour = FAKE_HOUR;
    dt.min = FAKE_MINUTE;
    dt.sec = 0;
    dt.dotw = 0;
    
    rtc_set_datetime(&dt);
    printf("[NTP] FAKE TIME set: %04d-%02d-%02d %02d:%02d:00\n",
           FAKE_YEAR, FAKE_MONTH, FAKE_DAY, FAKE_HOUR, FAKE_MINUTE);
#else
    const int TIMEZONE_OFFSET_HOURS = -3;
    time_t timestamp = sec + (TIMEZONE_OFFSET_HOURS * 3600);
    struct tm *timeinfo = gmtime(&timestamp);
    
    if (timeinfo) {
        datetime_t dt;
        dt.year = timeinfo->tm_year + 1900;
        dt.month = timeinfo->tm_mon + 1;
        dt.day = timeinfo->tm_mday;
        dt.hour = timeinfo->tm_hour;
        dt.min = timeinfo->tm_min;
        dt.sec = timeinfo->tm_sec;
        dt.dotw = timeinfo->tm_wday;
        
        rtc_set_datetime(&dt);
        printf("[NTP] RTC set: %04d-%02d-%02d %02d:%02d:%02d\n",
               dt.year, dt.month, dt.day, dt.hour, dt.min, dt.sec);
    } else {
        printf("[NTP] ERROR: Failed to convert timestamp\n");
    }
#endif
    
    TimeService::getInstance().markTimeSynced();
}

bool TimeService::init() {
    if (initialized_) {
        return true;
    }

    rtc_init();
    
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "time.google.com");
    sntp_setservername(1, "pool.ntp.org");
    sntp_setservername(2, "time.nist.gov");
    sntp_init();
    
    initialized_ = true;
    printf("[Time] Service initialized\n");
    return true;
}

std::string TimeService::getTimeString() {
    if (!initialized_) {
        return "00:00:00";
    }

    datetime_t dt;
    if (!rtc_get_datetime(&dt) || dt.year < 2020) {
        return "00:00:00";
    }

    time_synced_ = true;
    
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", dt.hour, dt.min, dt.sec);
    return std::string(buffer);
}

std::string TimeService::getDateString() {
    if (!initialized_) {
        return "00/00/0000";
    }

    datetime_t dt;
    if (!rtc_get_datetime(&dt) || dt.year < 2020) {
        return "00/00/0000";
    }

    time_synced_ = true;
    
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02d/%02d/%04d", dt.day, dt.month, dt.year);
    return std::string(buffer);
}

std::string TimeService::getDateTimeString() {
    if (!initialized_) {
        return "00/00/0000 00:00:00";
    }

    datetime_t dt;
    if (!rtc_get_datetime(&dt) || dt.year < 2020) {
        return "00/00/0000 00:00:00";
    }

    time_synced_ = true;
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%02d/%02d/%04d %02d:%02d:%02d", 
             dt.day, dt.month, dt.year, dt.hour, dt.min, dt.sec);
    return std::string(buffer);
}

bool TimeService::isTimeSynced() {
    return time_synced_;
}

void TimeService::markTimeSynced() {
    time_synced_ = true;
}

bool TimeService::extractHourMinute(const std::string& time_str, int& hour, int& min) {
    if (sscanf(time_str.c_str(), "%d:%d", &hour, &min) == 2) {
        return true;
    }
    return false;
}