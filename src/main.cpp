#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/cyw43_arch.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <event_groups.h>
#include <semphr.h>
#include <lvgl.h>

#include "core/state_machine.hpp"
#include "tasks/task_lvgl.hpp"
#include "tasks/task_qr_reader.hpp"
#include "tasks/task_qr_validator.hpp"
#include "tasks/task_wifi_time_init.hpp"
#include "tasks/task_network.hpp"
#include "ui/screens/welcome_screen.hpp"
#include "services/time_service.hpp"
#include "services/fingerprint_service.hpp"

#include "adapters/lvgl/lvgl_display_adapter.hpp"
#include "adapters/lvgl/lvgl_touch_adapter.hpp"
#include "drivers/display/st7796/st7796_driver.hpp"
#include "drivers/touch/ft6336u/ft6336u_driver.hpp"
#include "drivers/qr_code_reader/gm67/gm67_driver.hpp"

#include "hal/rp2040/hal_spi_rp2040.hpp"
#include "hal/rp2040/hal_i2c_rp2040.hpp"
#include "hal/rp2040/hal_uart_rp2040.hpp"

#include "config/pin_config.hpp"
#include "config/display_config.hpp"
#include "config/qr_code_reader_config.hpp"
#include "config/fingerprint_config.hpp"

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
    printf("STACK OVERFLOW in task: %s\n", pcTaskName);
    while(1) {
        tight_loop_contents();
    }
}

extern "C" void vApplicationMallocFailedHook(void) {
    printf("MALLOC FAILED - Out of heap memory!\n");
    while(1) {
        tight_loop_contents();
    }
}

#define LVGL_BUFFER_SIZE (DISP_HOR_RES * LVGL_BUFFER_LINES)
static lv_color_t lvgl_buf1[LVGL_BUFFER_SIZE];

EventGroupHandle_t wifi_time_ready_group_event = NULL;

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    printf("\nStart System\n");

    lv_init();

    HAL_SPI_RP2040 *spi_hal_display = new HAL_SPI_RP2040(spi0, DISPLAY_PIN_MOSI, 0xFF, DISPLAY_PIN_SCK, DISPLAY_PIN_CS, DISPLAY_PIN_DC, DISPLAY_PIN_RST);

    ST7796Driver *display_driver = new ST7796Driver(spi_hal_display, DISP_HOR_RES, DISP_VER_RES);

    if (!display_driver->init()) {
        printf("ERROR: Failed to initialize display driver!\n");
        return -1;
    }

    display_driver->setRotation(DISP_ROTATION);

    LVGLDisplayAdapter *display_adapter = new LVGLDisplayAdapter(display_driver, lvgl_buf1, nullptr, LVGL_BUFFER_SIZE);
    display_adapter->registerDisplay();

    HAL_I2C_RP2040 *i2c_hal_touch = new HAL_I2C_RP2040(TOUCH_I2C_INSTANCE, TOUCH_PIN_SDA, TOUCH_PIN_SCL);

    if (!i2c_hal_touch->init(TOUCH_I2C_SPEED)) {
        printf("ERROR: Failed to initialize I2C for touch!\n");
        return -1;
    }

    FT6336U_Driver *touch_driver = new FT6336U_Driver(i2c_hal_touch, TOUCH_I2C_ADDR, TOUCH_WIDTH, TOUCH_HEIGHT, TOUCH_PIN_RST, TOUCH_PIN_INT);

    if (!touch_driver->init()) {
        printf("ERROR: Failed to initialize touch driver!\n");
        return -1;
    }

    LVGLTouchAdapter *touch_adapter = new LVGLTouchAdapter(touch_driver, DISP_HOR_RES, DISP_VER_RES, DISP_ROTATION);
    touch_adapter->registerInputDevice();

    HAL_UART_RP2040 *uart_hal_qr = new HAL_UART_RP2040(QR_UART_INSTANCE, QR_SCANNER_PIN_TX, QR_SCANNER_PIN_RX);

    if (!uart_hal_qr->init(QR_BAUDRATE)) {
        printf("ERROR: Failed to initialize UART for QR reader!\n");
        return -1;
    }

    GM67_Driver *qr_driver = new GM67_Driver(uart_hal_qr);

    if (!qr_driver->init()) {
        printf("ERROR: Failed to initialize QR driver!\n");
        return -1;
    }

    auto& fingerprintService = FingerprintService::getInstance();
    if (!fingerprintService.init()) {
        printf("ERROR: Failed to initialize fingerprint sensor!\n");
        return -1;
    }

    QueueHandle_t qr_data_queue = xQueueCreate(5, 512);
    if (qr_data_queue == NULL) {
        printf("ERROR: Failed to create QR data queue!\n");
        return -1;
    }

    static QRReaderParams qr_reader_params = {.qr_driver = qr_driver, .qr_data_queue = qr_data_queue};
    static QRValidatorParams qr_validator_params = {.qr_data_queue = qr_data_queue};

    wifi_time_ready_group_event = xEventGroupCreate();
    if (wifi_time_ready_group_event == NULL) {
        printf("ERROR: Failed to create WiFi/time event group!\n");
        return -1;
    }

    xEventGroupSetBits(wifi_time_ready_group_event, LVGL_READY_BIT);

    BaseType_t result;

    result = xTaskCreate(task_wifi_time_init, "WiFi_Time_Init", 2048, nullptr, 5, nullptr);
    if (result != pdPASS) {
        printf("ERROR: Failed to create WiFi/Time Init task!\n");
        return -1;
    }

    result = xTaskCreate(task_network, "Network", 3072, nullptr, 2, nullptr);
    if (result != pdPASS) {
        printf("ERROR: Failed to create Network task!\n");
        return -1;
    }

    result = xTaskCreate(task_lvgl, "LVGL", 3072, nullptr, 4, nullptr);
    if (result != pdPASS) {
        printf("ERROR: Failed to create LVGL task!\n");
        return -1;
    }

    result = xTaskCreate(task_qr_reader, "QR_Reader", 1536, &qr_reader_params, 3, nullptr);
    if (result != pdPASS) {
        printf("ERROR: Failed to create QR reader task!\n");
        return -1;
    }

    result = xTaskCreate(task_qr_validator, "QR_Validator", 2048, &qr_validator_params, 2, nullptr);
    if (result != pdPASS) {
        printf("ERROR: Failed to create QR validator task!\n");
        return -1;
    }

    printf("System ready\n");

    vTaskStartScheduler();

    printf("ERROR: Scheduler returned!\n");
    while (1) {
        tight_loop_contents();
    }

    return 0;
}