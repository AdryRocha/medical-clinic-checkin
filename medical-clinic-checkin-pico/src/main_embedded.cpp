#include <cstdint>
#include <cstdio>

// Pico SDK
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/watchdog.h"
#include "hardware/regs/watchdog.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Project headers
#include "services/logger_service.hpp"
#include "config/pin_config.hpp"
#include "config/display_config.hpp"
#include "boards/board_factory.hpp"
#include "drivers/display/st7796/st7796_driver.hpp"
#include "drivers/touch/ft6336u/ft6336u_driver.hpp"
#include "core/checkin_state_machine.hpp"
#include "lvgl.h"
#include "adapters/lvgl/lvgl_display_adapter.hpp"
#include "adapters/lvgl/lvgl_touch_adapter.hpp"
#include "tasks/task_ui.hpp"
#include "tasks/task_qr.hpp"
#include "tasks/task_sdcard.hpp"
#include "ui/screens/welcome_screen.hpp"
#include "ui/screens/qrcode_screen.hpp"

// Configuration defaults (ensure defined before use)
#ifndef LCD_WIDTH
#define LCD_WIDTH  480
#endif
#ifndef LCD_HEIGHT
#define LCD_HEIGHT 320
#endif
#define LVGL_BUF_SIZE (LCD_WIDTH * 120)

// LVGL buffer
static lv_color_t buf_lvgl[LVGL_BUF_SIZE];

// Global board/context and drivers (single definitions)
static BoardContext ctx;
static ST7796Driver* display_driver = nullptr;
static FT6336U_Driver* touch_driver = nullptr;
static LVGLDisplayAdapter* g_lv_disp_adapter = nullptr;
static LVGLTouchAdapter* g_lv_touch_adapter = nullptr;
// LVGL mutex used by UI and callbacks (single definition)
SemaphoreHandle_t g_lvgl_mutex = NULL;

// Task static TCBs and stacks (single definitions)
static StaticTask_t ui_task_tcb;
static StackType_t ui_task_stack[1024] __attribute__((aligned(8)));

static StaticTask_t fsm_task_tcb;
static StackType_t fsm_task_stack[1024] __attribute__((aligned(8)));

static StaticTask_t qr_task_tcb;
static StackType_t qr_task_stack[1024] __attribute__((aligned(8)));

static StaticTask_t touch_dbg_task_tcb;
static StackType_t touch_dbg_task_stack[512] __attribute__((aligned(8)));

static StaticTask_t sdcard_task_tcb;
static StackType_t sdcard_task_stack[512] __attribute__((aligned(8)));

// FreeRTOS debug stage marker (placed in .noinit)
extern "C" volatile uint32_t g_freertos_stage __attribute__((section(".noinit")));
volatile uint32_t g_freertos_stage __attribute__((section(".noinit"))) = 0U;

extern "C" void vFreeRTOSDebugStage( uint32_t stage ) {
    watchdog_hw->scratch[0] = 0xF00D0000U | ( stage & 0xFFFFU );
}

static inline void boot_stage(uint32_t tag) {
    watchdog_hw->scratch[0] = tag;
}

// Small boot pattern (keeps original behavior)
static void draw_boot_pattern(ST7796Driver* drv) {
    if (!drv) return;
    const uint16_t w = LCD_WIDTH;
    const uint16_t h = LCD_HEIGHT;
    const uint16_t bar = w / 4;
    drv->fillRect(0,         0, bar, h, 0xF800);
    drv->fillRect(bar,       0, bar, h, 0x07E0);
    drv->fillRect(bar * 2,   0, bar, h, 0x001F);
    drv->fillRect(bar * 3,   0, w - bar * 3, h, 0xFFFF);
}

// Minimal welcome/qrcode wrappers
static void safe_welcome_screen_show() { welcome_screen_show(); }
static void safe_qrcode_screen_show(const char* s) { qrcode_screen_show(s); }

// Task entry implementations (minimal, keep alive)
static void fsm_task_entry(void* arg) {
    (void)arg;
    Logger::info("[FSM] Task FSM iniciada");
    CheckinStateMachine::instance().process_loop();
    for(;;) vTaskDelay(pdMS_TO_TICKS(1000));
}

static void touch_debug_task_entry(void* arg) {
    (void)arg;
    Logger::info("[TOUCH_DBG] Task de diagnóstico iniciada");
    for (;;) {
        int irq_state = -1;
        if (ctx.gpio) irq_state = ctx.gpio->read(TOUCH_PIN_INT) ? 1 : 0;
        uint8_t td_status = 0xFF;
        if (touch_driver) td_status = touch_driver->getTdStatus();
        Logger::info("[TOUCH_DBG] IRQ=%d TD_STATUS=0x%02X", irq_state, td_status);
        static TickType_t last_log = 0;
        if ((xTaskGetTickCount() - last_log) > pdMS_TO_TICKS(2000)) {
            TaskHandle_t this_task = xTaskGetCurrentTaskHandle();
            UBaseType_t stack_min = uxTaskGetStackHighWaterMark(this_task);
            Logger::info("[TOUCH_DBG] loop ativo, heap livre: %u, stack min: %u words", xPortGetFreeHeapSize(), (unsigned)stack_min);
            last_log = xTaskGetTickCount();
        }
        if (touch_driver && ((td_status & 0x0F) > 0)) {
            TouchPoint tp;
            if (touch_driver->readPoint(&tp) && tp.valid) {
                Logger::info("[TOUCH_DBG] raw=(%u,%u)", (unsigned)tp.x, (unsigned)tp.y);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// System setup (keeps original initialization flow)
static void setup_system() {
    gpio_init(DISPLAY_PIN_BL);
    gpio_set_dir(DISPLAY_PIN_BL, GPIO_OUT);
    gpio_put(DISPLAY_PIN_BL, 1);

    stdio_init_all();
    absolute_time_t usb_deadline = make_timeout_time_ms(3000);
    while (!stdio_usb_connected() && (absolute_time_diff_us(get_absolute_time(), usb_deadline) > 0)) {
        sleep_ms(10);
    }
    sleep_ms(1500);

    uint32_t scratch = watchdog_hw->scratch[0];
    if ((scratch & 0xFFFF0000U) == 0xF00D0000U) {
        printf("[BOOT] freertos_stage_prev=%lu\n", (unsigned long)(scratch & 0xFFFFU));
    } else {
        printf("[BOOT] freertos_stage_prev=%lu\n", (unsigned long)g_freertos_stage);
    }
    watchdog_hw->scratch[0] = 0U;
    g_freertos_stage = 0U;
    fflush(stdout);

    printf("[BOOT] reset_reason(raw)=0x%08lx\n", (unsigned long)watchdog_hw->reason);
    fflush(stdout);

    if (watchdog_caused_reboot()) Logger::warn("[BOOT] Watchdog reboot detected.");
    sleep_ms(3000);
    Logger::info("=== TOTEM MEDICO START ===");

    ctx = BoardFactory::create_pico2w();

    if (ctx.spi_display) {
        ctx.spi_display->init(DISPLAY_BAUDRATE);
        display_driver = new ST7796Driver(ctx.spi_display, ctx.gpio, DISPLAY_PIN_CS, DISPLAY_PIN_DC, DISPLAY_PIN_RST, DISPLAY_PIN_BL, LCD_WIDTH, LCD_HEIGHT);
        display_driver->init();
        display_driver->setRotation(DISP_ROTATION);
        display_driver->fillScreen(0x0000);
        draw_boot_pattern(display_driver);
        Logger::info("[BOOT] Display Hardware OK.");
    }

    touch_driver = new FT6336U_Driver(ctx.i2c, ctx.gpio, 0x38, TOUCH_PIN_INT, TOUCH_PIN_RST);
    bool touch_ok = false;
    if (touch_driver) touch_ok = touch_driver->init();
    if (touch_ok) Logger::info("[BOOT] Touch initialized");

    Logger::info("[BOOT] Inicializando LVGL...");
    lv_init();
    Logger::info("[BOOT] LVGL inicializado com sucesso.");

    g_lv_disp_adapter = new LVGLDisplayAdapter(display_driver, (uint8_t*)buf_lvgl, nullptr, LVGL_BUF_SIZE);
    g_lv_disp_adapter->registerDisplay();
    if (touch_ok) {
        g_lv_touch_adapter = new LVGLTouchAdapter(touch_driver, LCD_WIDTH, LCD_HEIGHT);
        g_lv_touch_adapter->registerInputDevice();
    }

    safe_welcome_screen_show();
    lv_timer_handler();
    lv_refr_now(nullptr);
    Logger::info("[BOOT] Setup concluído!");
}

int main() {
    #if DEBUG_WDT == 0
    watchdog_disable();
    #endif

    setvbuf(stdout, NULL, _IONBF, 0);

    setup_system();
    Logger::info("[MAIN] setup_system() concluído.");

    if (!display_driver || !touch_driver) {
        Logger::error("[MAIN] ERRO: display_driver ou touch_driver NULL!");
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    Logger::info("[MAIN] FreeRTOS heap livre antes da TaskUI: %u bytes", (unsigned)xPortGetFreeHeapSize());

    static TaskUI uiTask(display_driver, touch_driver);
    Logger::info("[MAIN] TaskUI construída.");

    boot_stage(0x00000005);
    TaskHandle_t ui_handle = xTaskCreateStatic(
        TaskUI::taskEntry,
        "UI_Task",
        1024,
        &uiTask,
        4,
        ui_task_stack,
        &ui_task_tcb
    );
    if (ui_handle == nullptr) { Logger::error("[MAIN] ERRO: Falha ao criar TaskUI (static)!"); while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); } }

    TaskHandle_t fsm_handle = xTaskCreateStatic(fsm_task_entry, "FSM_Task", 1024, nullptr, 2, fsm_task_stack, &fsm_task_tcb);
    if (fsm_handle == nullptr) { Logger::error("[MAIN] ERRO: Falha ao criar FSM_Task!"); while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); } }

    static TaskQR qrTask(ctx.uart_qr, &CheckinStateMachine::instance());
    TaskHandle_t qr_handle = xTaskCreateStatic([](void* arg){ static_cast<TaskQR*>(arg)->run(); }, "QR_Task", 1024, &qrTask, 3, qr_task_stack, &qr_task_tcb);
    if (qr_handle == nullptr) { Logger::error("[MAIN] ERRO: Falha ao criar QR_Task!"); while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); } }

    TaskHandle_t touch_dbg_handle = xTaskCreateStatic(touch_debug_task_entry, "TouchDbg", 512, nullptr, 1, touch_dbg_task_stack, &touch_dbg_task_tcb);
    if (touch_dbg_handle == nullptr) { Logger::error("[MAIN] ERRO: Falha ao criar TouchDbg Task!"); while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); } }

    TaskHandle_t sdcard_handle = xTaskCreateStatic(task_sdcard_entry, "SDCard_Task", 512, nullptr, 1, sdcard_task_stack, &sdcard_task_tcb);
    if (sdcard_handle == nullptr) { Logger::error("[MAIN] ERRO: Falha ao criar SDCard_Task!"); } else { Logger::info("[MAIN] SDCard_Task criada com handle=%p", (void*)sdcard_handle); }

    static StaticTask_t heapmon_task_tcb;
    static StackType_t heapmon_task_stack[256] __attribute__((aligned(8)));
    auto heapmon_task_entry = [](void* arg){ (void)arg; for(;;) { size_t min_heap = xPortGetFreeHeapSize(); Logger::info("[HEAPMON] Heap livre: %u bytes", (unsigned)min_heap); vTaskDelay(pdMS_TO_TICKS(5000)); } };
    TaskHandle_t heapmon_handle = xTaskCreateStatic(heapmon_task_entry, "HeapMon", 256, nullptr, 1, heapmon_task_stack, &heapmon_task_tcb);
    if (heapmon_handle == nullptr) { Logger::error("[MAIN] ERRO: Falha ao criar HeapMon Task!"); }

    Logger::info("[MAIN] Iniciando Scheduler FreeRTOS...");
    vTaskStartScheduler();

    Logger::error("[BOOT] Scheduler retornou (erro)");
    while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    return 0;
}

extern "C" {
    void vApplicationTickHook(void) { }

    void vApplicationStackOverflowHook(TaskHandle_t x, char* name) {
        (void)x;
        printf("OVF: %s\n", name ? name : "<null>");
        fflush(stdout);
        taskDISABLE_INTERRUPTS();
        while (1) { }
    }

    void vApplicationMallocFailedHook(void) {
        printf("MALLOC FAIL\n");
        fflush(stdout);
        taskDISABLE_INTERRUPTS();
        while (1) { }
    }

    void vAssertCalled(const char* file, int line) {
        (void)file;
        watchdog_hw->scratch[0] = 0xA55E0000u | (line & 0xFFFFu);
        #if DEBUG_WDT == 1
            watchdog_enable(200, 1); // reset rápido para expor o line number no próximo boot
        #endif
        while (1) { tight_loop_contents(); }
    }

    static void HardFault_C_Handler(uint32_t *stack) {
        uint32_t pc   = stack[6];
        uint32_t lr   = stack[5];
        uint32_t cfsr = *(volatile uint32_t *)0xE000ED28;
        uint32_t hfsr = *(volatile uint32_t *)0xE000ED2C;

        /* Save signature + CFSR on scratch[0], PC on scratch[1] for post-mortem. */
        watchdog_hw->scratch[0] = 0xFA170000u | ( cfsr & 0xFFFFu );
        watchdog_hw->scratch[1] = pc;

        printf("HARDFAULT pc=0x%08lx lr=0x%08lx cfsr=0x%08lx hfsr=0x%08lx\n",
               (unsigned long)pc, (unsigned long)lr,
               (unsigned long)cfsr, (unsigned long)hfsr);
        fflush(stdout);
        #if DEBUG_WDT == 1
            watchdog_enable(150, 1);
        #endif
        while (1) { }
    }

    __attribute__((naked)) void HardFault_Handler(void) {
        __asm volatile(
            "tst lr, #4\n"
            "ite eq\n"
            "mrseq r0, msp\n"
            "mrsne r0, psp\n"
            "b HardFault_C_Handler\n"
        );
    }

    void MemManage_Handler(void) {
        volatile uint32_t *cfsr = (uint32_t *)0xE000ED28;
        volatile uint32_t *mmfar = (uint32_t *)0xE000ED34;
        printf("MEMMANAGE\n");
        printf("CFSR=0x%08lx MMFAR=0x%08lx\n", (unsigned long)*cfsr, (unsigned long)*mmfar);
        fflush(stdout);
        while (1) { }
    }

    void BusFault_Handler(void) {
        volatile uint32_t *cfsr = (uint32_t *)0xE000ED28;
        volatile uint32_t *bfar = (uint32_t *)0xE000ED38;
        printf("BUSFAULT\n");
        printf("CFSR=0x%08lx BFAR=0x%08lx\n", (unsigned long)*cfsr, (unsigned long)*bfar);
        fflush(stdout);
        while (1) { }
    }

    void UsageFault_Handler(void) {
        volatile uint32_t *cfsr = (uint32_t *)0xE000ED28;
        printf("USAGEFAULT\n");
        printf("CFSR=0x%08lx\n", (unsigned long)*cfsr);
        fflush(stdout);
        while (1) { }
    }

    void SecureFault_Handler(void) {
        /* ARMv8-M SecureFault status regs (if implemented/enabled):
         * SFSR @ 0xE000EDE4, SFAR @ 0xE000EDE8 */
        volatile uint32_t *sfsr = (uint32_t *)0xE000EDE4;
        volatile uint32_t *sfar = (uint32_t *)0xE000EDE8;
        printf("SECUREFAULT\n");
        printf("SFSR=0x%08lx SFAR=0x%08lx\n", (unsigned long)*sfsr, (unsigned long)*sfar);
        fflush(stdout);
        while (1) { }
    }

    void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                       StackType_t **ppxIdleTaskStackBuffer,
                                       uint32_t *pulIdleTaskStackSize) {
        static StaticTask_t idleTaskTCB;
        static StackType_t idleTaskStack[configMINIMAL_STACK_SIZE] __attribute__((aligned(8)));
        *ppxIdleTaskTCBBuffer = &idleTaskTCB;
        *ppxIdleTaskStackBuffer = idleTaskStack;
        *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
    }

    void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                        StackType_t **ppxTimerTaskStackBuffer,
                                        uint32_t *pulTimerTaskStackSize) {
        static StaticTask_t timerTaskTCB;
        static StackType_t timerTaskStack[configTIMER_TASK_STACK_DEPTH] __attribute__((aligned(8)));
        *ppxTimerTaskTCBBuffer = &timerTaskTCB;
        *ppxTimerTaskStackBuffer = timerTaskStack;
        *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
    }
}