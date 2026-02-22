#include "logger_service.hpp"

// Ponteiro do mutex
SemaphoreHandle_t LoggerService::mutex_ = nullptr;

void LoggerService::init() {
    if (!mutex_) {
        mutex_ = xSemaphoreCreateMutex();
    }
}

const char* LoggerService::level_to_str(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:  return "[INFO] ";
        case LogLevel::WARN:  return "[WARN] ";
        case LogLevel::ERROR: return "[ERROR]";
    }
    return "[UNKN]";
}

void LoggerService::log(LogLevel level, const char* fmt, ...) {
    if (!mutex_) return;

    if (xSemaphoreTake(mutex_, portMAX_DELAY) == pdTRUE) {
        printf("%s ", level_to_str(level));

        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);

        printf("\n");
        fflush(stdout);

        xSemaphoreGive(mutex_);
    }
}
