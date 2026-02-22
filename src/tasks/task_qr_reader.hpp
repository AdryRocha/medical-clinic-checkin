#ifndef TASK_QR_READER_HPP
#define TASK_QR_READER_HPP

#include <FreeRTOS.h>
#include <queue.h>

class GM67_Driver;

struct QRReaderParams {
    GM67_Driver* qr_driver;
    QueueHandle_t qr_data_queue;
};

/**
 * @brief QR Reader task - reads QR codes and sends to validator
 * @param params Pointer to QRReaderParams struct
 */
void task_qr_reader(void* params);

#endif // TASK_QR_READER_HPP