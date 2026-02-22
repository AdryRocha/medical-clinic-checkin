#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "drivers/qr_code_reader/gm67/gm67_driver.hpp"
#include "hal/rp2040/hal_uart_rp2040.hpp"
#include "config/pin_config.hpp"
#include "config/qr_code_reader_config.hpp"

#define QR_TX_PIN           QR_SCANNER_PIN_TX
#define QR_RX_PIN           QR_SCANNER_PIN_RX
#define LED_PIN             STATUS_PIN_LED

static HAL_UART_RP2040* uart_hal = nullptr;
static GM67_Driver* qr_scanner = nullptr;

void qr_scan_callback(const std::string& data) {
    printf("\n>>> QR: %s\n", data.c_str());
}

void qr_scanner_task(void *pvParameters) {
    printf("Initializing scanner...\n");
    
    uart_hal = new HAL_UART_RP2040(QR_UART_INSTANCE, QR_TX_PIN, QR_RX_PIN);
    if (!uart_hal->init(QR_BAUDRATE, QR_DATA_BITS, QR_STOP_BITS, QR_PARITY)) {
        printf("ERROR: UART init failed!\n");
        vTaskDelete(NULL);
        return;
    }
    
    qr_scanner = new GM67_Driver(uart_hal);
    if (!qr_scanner->init()) {
        printf("ERROR: Scanner init failed!\n");
        vTaskDelete(NULL);
        return;
    }
    
    qr_scanner->setScanCallback(qr_scan_callback);
    vTaskDelay(pdMS_TO_TICKS(100));
    qr_scanner->setContinuousMode();
    vTaskDelay(pdMS_TO_TICKS(100));
    qr_scanner->enableScan(true);
    
    printf("Ready!\n\n");
    
    while (true) {
        qr_scanner->process();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void blink_task(void *pvParameters) {
    while (true) {
        gpio_put(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_put(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    
    printf("\n=== QR Scanner Test ===\n");
    printf("UART%d @ %d baud (TX=%d, RX=%d)\n\n", 
           QR_UART_INSTANCE == uart0 ? 0 : 1, QR_BAUDRATE, QR_TX_PIN, QR_RX_PIN);
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    xTaskCreate(qr_scanner_task, "QR", 1024, NULL, 2, NULL);
    xTaskCreate(blink_task, "Blink", 128, NULL, 1, NULL);
    
    vTaskStartScheduler();
    
    while (1) { }
    return 0;
}
