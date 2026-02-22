#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// LVGL
#include "lvgl.h"
#include "demos/lv_demos.h"

// Configuration
#include "config/pin_config.hpp"
#include "config/display_config.hpp"

// HAL and Drivers
#include "hal/rp2040/hal_spi_rp2040.hpp"
#include "hal/rp2040/hal_i2c_rp2040.hpp"
#include "drivers/display/st7796/st7796_driver.hpp"
#include "drivers/touch/ft6336u/ft6336u_driver.hpp"
#include "adapters/lvgl/lvgl_display_adapter.hpp"
#include "adapters/lvgl/lvgl_touch_adapter.hpp"

// LVGL buffers
static lv_color_t lvgl_buf1[DISP_HOR_RES * LVGL_BUFFER_LINES];
#if LVGL_USE_DOUBLE_BUFFER
static lv_color_t lvgl_buf2[DISP_HOR_RES * LVGL_BUFFER_LINES];
#endif

// Hardware instances
static HAL_SPI_RP2040* hal_spi = nullptr;
static HAL_I2C_RP2040* hal_i2c = nullptr;
static ST7796Driver* display = nullptr;
static FT6336U_Driver* touch = nullptr;
static LVGLDisplayAdapter* lvgl_adapter = nullptr;
static LVGLTouchAdapter* lvgl_touch_adapter = nullptr;

// FreeRTOS synchronization
static SemaphoreHandle_t lvgl_mutex = nullptr;

/**
 * @brief LVGL timer handler task
 * Calls lv_timer_handler() periodically to update the GUI
 */
void lvgl_task(void* param) {
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t frequency = pdMS_TO_TICKS(5); // 5ms = ~200Hz

    while (1) {
        if (xSemaphoreTake(lvgl_mutex, portMAX_DELAY) == pdTRUE) {
            lv_timer_handler();
            xSemaphoreGive(lvgl_mutex);
        }

        vTaskDelayUntil(&last_wake_time, frequency);
    }
}

/**
 * @brief Display initialization task
 * Sets up hardware and LVGL, then starts the demo
 */
void display_init_task(void* param) {
    printf("Initializing display hardware...\n");

    hal_spi = new HAL_SPI_RP2040(
        spi0,
        DISPLAY_PIN_MOSI,
        0xFF,  // MISO não utilizado
        DISPLAY_PIN_SCK,
        DISPLAY_PIN_CS,
        DISPLAY_PIN_DC,
        DISPLAY_PIN_RST
    );

    display = new ST7796Driver(hal_spi, DISP_HOR_RES, DISP_VER_RES);
    
    if (!display->init()) {
        printf("ERROR: Display initialization failed!\n");
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    printf("Display initialized\n");

    display->setRotation(DISP_ROTATION);
    hal_i2c = new HAL_I2C_RP2040(i2c1, TOUCH_PIN_SDA, TOUCH_PIN_SCL);
    
    if (!hal_i2c->init(TOUCH_I2C_SPEED)) {
        printf("ERROR: I2C initialization failed!\n");
    } else {
        touch = new FT6336U_Driver(hal_i2c, TOUCH_I2C_ADDR, TOUCH_WIDTH, TOUCH_HEIGHT,
                                   TOUCH_PIN_RST, TOUCH_PIN_INT);
        
        if (!touch->init()) {
            printf("WARNING: Touch initialization failed\n");
            delete touch;
            touch = nullptr;
        }
    }

    lv_init();

    lvgl_mutex = xSemaphoreCreateMutex();
    if (lvgl_mutex == nullptr) {
        printf("ERROR: Failed to create LVGL mutex!\n");
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    lvgl_adapter = new LVGLDisplayAdapter(
        display,
        lvgl_buf1,
#if LVGL_USE_DOUBLE_BUFFER
        lvgl_buf2,
#else
        nullptr,
#endif
        DISP_HOR_RES * LVGL_BUFFER_LINES
    );
    lvgl_adapter->registerDisplay();
    if (touch != nullptr) {
        lvgl_touch_adapter = new LVGLTouchAdapter(
            touch,
            DISP_HOR_RES,
            DISP_VER_RES,
            DISP_ROTATION
        );
        lvgl_touch_adapter->registerInputDevice();
    }

    if (xSemaphoreTake(lvgl_mutex, portMAX_DELAY) == pdTRUE) {
        lv_demo_widgets();
        xSemaphoreGive(lvgl_mutex);
    }

    xTaskCreate(
        lvgl_task,
        "LVGL Task",
        2048,
        nullptr,
        tskIDLE_PRIORITY + 2,
        nullptr
    );

    vTaskDelete(nullptr);
}

/**
 * @brief Heartbeat task for debugging
 * Blinks the onboard LED to indicate the system is running
 */
void heartbeat_task(void* param) {
    const uint LED_PIN = STATUS_PIN_LED;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1) {
        gpio_put(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_put(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(900));
    }
}

/**
 * @brief Main entry point
 */
int main() {
    stdio_init_all();
    
    sleep_ms(2000);
    
    printf("\n========================================\n");
    printf("LVGL + FreeRTOS Demo\n");
    printf("========================================\n");

    xTaskCreate(
        display_init_task,
        "Display Init",
        2048,
        nullptr,
        tskIDLE_PRIORITY + 1,
        nullptr
    );

    xTaskCreate(
        heartbeat_task,
        "Heartbeat",
        256,
        nullptr,
        tskIDLE_PRIORITY,
        nullptr
    );

    printf("Starting FreeRTOS scheduler...\n");
    vTaskStartScheduler();

    printf("ERROR: Scheduler failed to start!\n");
    while (1) {
        tight_loop_contents();
    }

    return 0;
}

/**
 * @brief FreeRTOS malloc failed hook
 */
extern "C" void vApplicationMallocFailedHook(void) {
    printf("ERROR: FreeRTOS malloc failed!\n");
    while (1) {
        tight_loop_contents();
    }
}

/**
 * @brief FreeRTOS stack overflow hook
 */
extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
    printf("ERROR: Stack overflow in task: %s\n", pcTaskName);
    while (1) {
        tight_loop_contents();
    }
}
