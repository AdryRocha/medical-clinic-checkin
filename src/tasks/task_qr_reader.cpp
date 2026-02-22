#include "tasks/task_qr_reader.hpp"
#include "core/state_machine.hpp"
#include "drivers/qr_code_reader/gm67/gm67_driver.hpp"
#include "ui/screens/error_screen.hpp"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdio.h>
#include <string.h>

static QueueHandle_t global_qr_queue = nullptr;

void qr_scan_callback(const std::string& data) {
    StateMachine::State current_state = StateMachine::getInstance().getState();
    if (current_state != StateMachine::State::IDLE) {
        return;
    }
    
    printf("[QR Task] Scanned: %s\n", data.c_str());
    
    if (xQueueSend(global_qr_queue, data.c_str(), pdMS_TO_TICKS(100)) == pdTRUE) {
        StateMachine::getInstance().setState(StateMachine::State::VALIDATING);
    } else {
        printf("[QR Task] ERROR: Queue full!\n");
        error_screen_update(
            "Sistema ocupado",
            "Aguarde um momento e tente novamente"
        );
        StateMachine::getInstance().setState(StateMachine::State::ERROR);
        vTaskDelay(pdMS_TO_TICKS(2000));
        StateMachine::getInstance().setState(StateMachine::State::IDLE);
    }
}

void task_qr_reader(void* params) {
    QRReaderParams* qr_params = static_cast<QRReaderParams*>(params);
    
    if (!qr_params || !qr_params->qr_driver || !qr_params->qr_data_queue) {
        printf("[QR Task] ERROR: Invalid parameters!\n");
        vTaskDelete(NULL);
        return;
    }
    
    GM67_Driver* qr_driver = qr_params->qr_driver;
    global_qr_queue = qr_params->qr_data_queue;
    
    printf("[QR Task] Started\n");
    
    qr_driver->setScanCallback(qr_scan_callback);
    vTaskDelay(pdMS_TO_TICKS(100));
    qr_driver->setContinuousMode();
    vTaskDelay(pdMS_TO_TICKS(100));
    qr_driver->enableScan(true);
    
    printf("[QR Task] Ready to scan\n");
    
    while (1) {
        qr_driver->process();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}