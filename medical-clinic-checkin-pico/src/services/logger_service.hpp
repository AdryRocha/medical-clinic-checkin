#pragma once
#ifndef LOGGER_SERVICE_HPP
#define LOGGER_SERVICE_HPP

#include <cstdio>
#include <cstdarg>
#include <string>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "semphr.h"

enum class LogLevel { INFO, WARN, ERROR };

class LoggerService {
public:
    static void init();
    
    // Método base
    static void log(LogLevel level, const char* fmt, ...);

    // Helpers estáticos (Resolvem o erro 'Logger::info')
    template<typename... Args>
    static void info(const char* fmt, Args... args) {
        log(LogLevel::INFO, fmt, args...);
    }

    template<typename... Args>
    static void warn(const char* fmt, Args... args) {
        log(LogLevel::WARN, fmt, args...);
    }

    template<typename... Args>
    static void error(const char* fmt, Args... args) {
        log(LogLevel::ERROR, fmt, args...);
    }

private:
    static SemaphoreHandle_t mutex_;
    static const char* level_to_str(LogLevel level);
};

// O ALIAS MÁGICO QUE FALTAVA
using Logger = LoggerService; 

// Macros (mantidas para compatibilidade)
#define LOGGER_INFO(fmt, ...)  LoggerService::log(LogLevel::INFO,  fmt, ##__VA_ARGS__)
#define LOGGER_WARN(fmt, ...)  LoggerService::log(LogLevel::WARN,  fmt, ##__VA_ARGS__)
#define LOGGER_ERROR(fmt, ...) LoggerService::log(LogLevel::ERROR, fmt, ##__VA_ARGS__)

#endif // LOGGER_SERVICE_HPP