#include "services/fingerprint_service.hpp"
#include "services/data_storage_service.hpp"
#include "drivers/fingerprint/r307s/r307s_driver.hpp"
#include "hal/rp2040/hal_uart_rp2040.hpp"
#include "ui/screens/fingerprint_operation_screen.hpp"
#include "config/pin_config.hpp"
#include "config/fingerprint_config.hpp"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "FreeRTOS.h"
#include "task.h"
#include <cstdio>

FingerprintService& FingerprintService::getInstance() {
    static FingerprintService instance;
    return instance;
}

FingerprintService::FingerprintService() 
    : fp_driver_(nullptr),
      uart_hal_(nullptr),
      is_initialized_(false) {
}

FingerprintService::~FingerprintService() {
    if (fp_driver_) {
        delete fp_driver_;
        fp_driver_ = nullptr;
    }
    if (uart_hal_) {
        delete uart_hal_;
        uart_hal_ = nullptr;
    }
}

bool FingerprintService::init() {
    if (is_initialized_) {
        return true;
    }
    
    uart_hal_ = new HAL_UART_RP2040(FP_UART_INSTANCE, FINGERPRINT_PIN_TX, FINGERPRINT_PIN_RX);
    if (!uart_hal_) {
        printf("[Fingerprint] ERROR: Failed to create UART HAL\n");
        return false;
    }
    
    if (!uart_hal_->init(FP_BAUDRATE, FP_DATA_BITS, FP_STOP_BITS, FP_PARITY)) {
        printf("[Fingerprint] ERROR: UART init failed\n");
        delete uart_hal_;
        uart_hal_ = nullptr;
        return false;
    }
    
    fp_driver_ = new R307S_Driver(uart_hal_);
    if (!fp_driver_) {
        printf("[Fingerprint] ERROR: Failed to create driver\n");
        delete uart_hal_;
        uart_hal_ = nullptr;
        return false;
    }
    
    if (!fp_driver_->init()) {
        printf("[Fingerprint] ERROR: Sensor init failed\n");
        delete fp_driver_;
        delete uart_hal_;
        fp_driver_ = nullptr;
        uart_hal_ = nullptr;
        return false;
    }
    
    uint16_t count = 0;
    fp_driver_->getTemplateCount(count);
    printf("[Fingerprint] Sensor ready (templates: %d)\n", count);
    
    is_initialized_ = true;
    return true;
}

bool FingerprintService::waitForFinger(int timeout_ms) {
    uint32_t start_ms = to_ms_since_boot(get_absolute_time());
    
    while (true) {
        FingerprintStatus status = fp_driver_->getImage();
        
        if (status == FingerprintStatus::OK) {
            return true;
        }
        
        uint32_t elapsed_ms = to_ms_since_boot(get_absolute_time()) - start_ms;
        if (elapsed_ms > timeout_ms) {
            printf("[Fingerprint] Timeout waiting for finger\n");
            return false;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

bool FingerprintService::waitForFingerRemoval(int timeout_ms) {
    uint32_t start_ms = to_ms_since_boot(get_absolute_time());
    
    while (true) {
        FingerprintStatus status = fp_driver_->getImage();
        
        if (status == FingerprintStatus::ERROR_NO_FINGER) {
            vTaskDelay(pdMS_TO_TICKS(500));
            return true;
        }
        
        uint32_t elapsed_ms = to_ms_since_boot(get_absolute_time()) - start_ms;
        if (elapsed_ms > timeout_ms) {
            return false;
        }
        
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

bool FingerprintService::enrollFingerprint(std::vector<uint8_t>& template_data,
                                          int timeout_per_scan_ms) {
    if (!is_initialized_) {
        printf("[Fingerprint] ERROR: Not initialized\n");
        return false;
    }
    
    printf("[Fingerprint] Starting enrollment...\n");
    
    if (!waitForFinger(timeout_per_scan_ms)) {
        return false;
    }
    
    FingerprintStatus status = fp_driver_->image2Tz(1);
    if (status != FingerprintStatus::OK) {
        printf("[Fingerprint] ERROR: First image failed (status: %d)\n", static_cast<int>(status));
        return false;
    }
    
    fingerprint_operation_screen_update_status("Primeira leitura OK!");
    fingerprint_operation_screen_set_step(2);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    if (!waitForFingerRemoval(5000)) {
        printf("[Fingerprint] WARNING: Finger not removed, continuing\n");
    }
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    fingerprint_operation_screen_set_step(3);
    fingerprint_operation_screen_update_status("Aguardando segunda leitura...");
    vTaskDelay(pdMS_TO_TICKS(100));
    
    if (!waitForFinger(timeout_per_scan_ms)) {
        return false;
    }
    
    status = fp_driver_->image2Tz(2);
    if (status != FingerprintStatus::OK) {
        printf("[Fingerprint] ERROR: Second image failed (status: %d)\n", static_cast<int>(status));
        return false;
    }
    
    status = fp_driver_->createModel();
    if (status != FingerprintStatus::OK) {
        printf("[Fingerprint] ERROR: Model creation failed (status: %d)\n", static_cast<int>(status));
        return false;
    }
    
    status = fp_driver_->storeModel(999);
    if (status != FingerprintStatus::OK) {
        printf("[Fingerprint] ERROR: Store model failed (status: %d)\n", static_cast<int>(status));
        return false;
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    status = fp_driver_->loadTemplate(999, 2);
    if (status != FingerprintStatus::OK) {
        printf("[Fingerprint] ERROR: Load template failed (status: %d)\n", static_cast<int>(status));
        fp_driver_->deleteModel(999);
        return false;
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    status = fp_driver_->downloadTemplate(2, template_data);
    if (status != FingerprintStatus::OK) {
        printf("[Fingerprint] ERROR: Download template failed (status: %d)\n", static_cast<int>(status));
        fp_driver_->deleteModel(999);
        return false;
    }
    
    fp_driver_->deleteModel(999);
    
    printf("[Fingerprint] Enrollment OK (%zu bytes)\n", template_data.size());
    return true;
}

bool FingerprintService::loadTemplateToSensor(const std::vector<uint8_t>& data, uint8_t slot) {
    if (!is_initialized_) {
        printf("[Fingerprint] ERROR: Not initialized\n");
        return false;
    }
    
    FingerprintStatus status = fp_driver_->uploadTemplate(slot, data);
    if (status != FingerprintStatus::OK) {
        printf("[Fingerprint] ERROR: Upload to slot %d failed (status: %d)\n", slot, static_cast<int>(status));
        return false;
    }
    
    return true;
}

bool FingerprintService::verifyFingerprint(int patient_id, 
                                          uint16_t& confidence,
                                          int timeout_ms) {
    if (!is_initialized_) {
        printf("[Fingerprint] ERROR: Not initialized\n");
        return false;
    }
    
    printf("[Fingerprint] Verifying patient %d...\n", patient_id);
    
    auto& storage = DataStorageService::getInstance();
    std::vector<uint8_t> stored_template;
    
    if (!storage.loadFingerprintTemplate(patient_id, stored_template)) {
        printf("[Fingerprint] ERROR: Failed to load template from SD\n");
        return false;
    }
    
    FingerprintStatus status = fp_driver_->uploadTemplate(2, stored_template);
    if (status != FingerprintStatus::OK) {
        printf("[Fingerprint] ERROR: Upload to sensor failed (status: %d)\n", static_cast<int>(status));
        return false;
    }
    
    if (!waitForFinger(timeout_ms)) {
        printf("[Fingerprint] ERROR: Timeout waiting for finger\n");
        return false;
    }
    
    status = fp_driver_->image2Tz(1);
    if (status != FingerprintStatus::OK) {
        printf("[Fingerprint] ERROR: Image processing failed (status: %d)\n", static_cast<int>(status));
        return false;
    }
    
    status = fp_driver_->compareTemplates(confidence);
    
    if (status == FingerprintStatus::OK) {
        printf("[Fingerprint] Match! Confidence: %d\n", confidence);
        return true;
    } else {
        printf("[Fingerprint] No match (confidence: %d)\n", confidence);
        return false;
    }
}

bool FingerprintService::clearSensorMemory() {
    if (!is_initialized_) {
        return false;
    }
    
    FingerprintStatus status = fp_driver_->emptyDatabase();
    if (status != FingerprintStatus::OK) {
        printf("[Fingerprint] ERROR: Failed to clear sensor (status: %d)\n", static_cast<int>(status));
        return false;
    }
    
    printf("[Fingerprint] Sensor memory cleared\n");
    return true;
}

bool FingerprintService::getTemplateCount(uint16_t& count) {
    if (!is_initialized_) {
        return false;
    }
    
    FingerprintStatus status = fp_driver_->getTemplateCount(count);
    return (status == FingerprintStatus::OK);
}