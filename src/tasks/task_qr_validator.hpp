#ifndef TASK_QR_VALIDATOR_HPP
#define TASK_QR_VALIDATOR_HPP

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

struct QRValidatorParams {
    QueueHandle_t qr_data_queue;
};

/**
 * @brief QR Validator task - parses JSON from QR codes and displays info
 * @param params Pointer to QRValidatorParams struct
 */
void task_qr_validator(void* params);

#endif // TASK_QR_VALIDATOR_HPP