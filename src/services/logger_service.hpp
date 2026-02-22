#ifndef LOGGER_SERVICE_HPP
#define LOGGER_SERVICE_HPP

#include <cstdio>
#include <cstdarg>
#include <mutex>
#include <string>

#include "pico/stdlib.h"   // para printf (USB serial)
#include "FreeRTOS.h"
#include "semphr.h"

enum class LogLevel {
    INFO,
    WARN,
    ERROR
};

class LoggerService {
public:
    static void init();
    static void log(LogLevel level, const char* fmt, ...);

private:
    static SemaphoreHandle_t mutex_;
    static const char* level_to_str(LogLevel level);
};

// ---------- MACROS FÁCEIS PARA USO ----------
#define LOGGER_INFO(fmt, ...)  LoggerService::log(LogLevel::INFO,  fmt, ##__VA_ARGS__)
#define LOGGER_WARN(fmt, ...)  LoggerService::log(LogLevel::WARN,  fmt, ##__VA_ARGS__)
#define LOGGER_ERROR(fmt, ...) LoggerService::log(LogLevel::ERROR, fmt, ##__VA_ARGS__)

#endif // LOGGER_SERVICE_HPP
