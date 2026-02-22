#include "logger_service.hpp"
#include "FreeRTOS.h"
#include "task.h"

// Ponteiro do mutex
SemaphoreHandle_t LoggerService::mutex_ = nullptr;

void LoggerService::init() {
    // Avoid creating FreeRTOS objects before the scheduler starts.
    // Mutex will be created lazily on first use once the scheduler is running.
    mutex_ = nullptr;
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
    // 1. Se o scheduler NÃO estiver rodando, imprime direto (thread-safe pois é single thread aqui)
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        printf("%s ", level_to_str(level));
        
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        
        printf("\n");
        fflush(stdout);
        return; 
    }

    // 2. If scheduler started, ensure mutex exists and use it
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        if (!mutex_) {
            // Create mutex now that kernel is up; if creation fails, fall back to direct print
            mutex_ = xSemaphoreCreateMutex();
            if (!mutex_) {
                printf("%s ", level_to_str(level));
                va_list args;
                va_start(args, fmt);
                vprintf(fmt, args);
                va_end(args);
                printf("\n");
                fflush(stdout);
                return;
            }
        }

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
        return;
    }
}
