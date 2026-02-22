/*
 * FreeRTOS V202111.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

/* Scheduler Related */
#define configUSE_PREEMPTION                    1
#define configUSE_TICKLESS_IDLE                 0
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0 /* Desabilitado para isolar start do scheduler */
#define configUSE_PASSIVE_IDLE_HOOK             0
#define configCPU_CLOCK_HZ                      ( ( unsigned long ) 125000000 ) /* RP2350 = 150MHz */
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                    32

/* The ARM_CM33_NTZ port in this repo does not provide the additional port*
 * macros required by the SMP FreeRTOS kernel (portGET_CORE_ID, task/isr locks,
 * interrupt mask helpers, etc). Keep this build single-core.
 *
 * If we want true multi-core scheduling on RP2350, we must switch to a port
 * that implements the SMP API, or add those port macros + core yield/locks.
 */
/* Force single-core to avoid SMP paths without a proper SMP port. */
#define configNUMBER_OF_CORES                   1
#define configNUM_CORES                         1
#if ( configNUMBER_OF_CORES != 1 )
    #error "Este firmware deve ser SINGLE-CORE. configNUMBER_OF_CORES != 1"
#endif

/* Not supported/used in single-core. Keep off to avoid pulling SMP-only APIs. */
#define configUSE_CORE_AFFINITY                  0
#define configMINIMAL_STACK_SIZE                ( configSTACK_DEPTH_TYPE ) 256 /* Aumentado para segurança */
#define configTICK_TYPE_WIDTH_IN_BITS           TICK_TYPE_WIDTH_32_BITS

/* SMP disallows this, and in general it's fine to leave it off for portability. */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  0

#define configIDLE_SHOULD_YIELD                 1

/* Synchronization Related */
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_APPLICATION_TASK_TAG          0
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_QUEUE_SETS                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_NEWLIB_REENTRANT              0
// todo need this for lwip FreeRTOS sys_arch to compile
#define configENABLE_BACKWARD_COMPATIBILITY     1
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5

/* Debug: trace task creation path */
#define FREERTOS_TASK_CREATE_DEBUG              0
#define FREERTOS_SKIP_TASK_CREATE_CRITICAL      0

/* System */
#define configSTACK_DEPTH_TYPE                  uint32_t
#define configMESSAGE_BUFFER_LENGTH_TYPE        size_t

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                  ( ( size_t ) ( 80 * 1024 ) ) // Aumente para 192KB!
#define configAPPLICATION_ALLOCATED_HEAP        0

/* Hook function related definitions. */
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         1

/* Software timer related definitions. */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            1024

/* Interrupt nesting behaviour configuration. */
/* Cortex-M (RP2350 / Cortex-M33): prioridades de IRQ precisam estar alinhadas
 * ao número de bits realmente implementados no NVIC.
 *
 * Regra prática FreeRTOS:
 * - Prioridade numérica menor = IRQ mais alta.
 * - IRQs com prioridade numérica < configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
 *   NÃO podem chamar APIs *FromISR.
 */
/* RP2350 (Cortex-M33) implementa 4 bits de prioridade. Forçamos 4 aqui para
 * evitar deslocamento incorreto nas prioridades do kernel/ISR caso o header
 * CMSIS não tenha definido __NVIC_PRIO_BITS. */
#undef configPRIO_BITS
#define configPRIO_BITS                         4

/* Nível mais baixo (menos urgente) possível no NVIC. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         ( ( 1U << configPRIO_BITS ) - 1U )

/* Valor recomendado pelo port RP2350: prioridade final (shiftada) = 16. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    1

/* Valores finais já shiftados para o formato do registrador do NVIC. */
#define configKERNEL_INTERRUPT_PRIORITY         ( 3 << 6 )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( 1 << 6 )

/* RP2040/RP2350 specific */
#define configSUPPORT_PICO_SYNC_INTEROP         1
#define configSUPPORT_PICO_TIME_INTEROP         1

/* Define to trap errors during development. */
#ifdef __cplusplus
extern "C" void vAssertCalled(const char* file, int line);
#else
void vAssertCalled(const char* file, int line);
#endif
#define configASSERT(x)                         if((x) == 0) { vAssertCalled(__FILE__, __LINE__); }

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskAbortDelay                 1
#define INCLUDE_xTaskGetHandle                  1
#define INCLUDE_xTaskResumeFromISR              1
#define INCLUDE_xQueueGetMutexHolder            1

/* A header file that defines trace macro can be included here. */

/* ==========================================
 * Configurações Específicas para Cortex-M33 (RP2350)
 * ========================================== */
#if defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)
    
    // Habilita FPU (Floating Point Unit) - Essencial para conta rápida
    #define configENABLE_FPU                        1
    
    // MPU (Memory Protection Unit) - Desabilitamos
    #define configENABLE_MPU                        0
    
    // TrustZone - Desabilitamos; kernel roda em secure-only
    #define configENABLE_TRUSTZONE                  0
    
    #define configRUN_FREERTOS_SECURE_ONLY          1

    // Minimal Stack Size precisa ser maior no M33
    #undef configMINIMAL_STACK_SIZE
    #define configMINIMAL_STACK_SIZE                ( 2048 )

    /* Se portCHECK_IF_IN_ISR não estiver definido pelo port, defina-o */
    #ifndef portCHECK_IF_IN_ISR
        #define portCHECK_IF_IN_ISR() ({ \
            uint32_t ulIPSR; \
            __asm volatile ("MRS %0, IPSR" : "=r" (ulIPSR) ); \
            ( ulIPSR != 0 ) ? 1 : 0; \
        })
    #endif

#endif 
        /* ==========================================================
 * MAPEAMENTO DE INTERRUPÇÕES (CRÍTICO PARA O BOOT)
 * Sem isso, vTaskStartScheduler() trava em HardFault ou Loop
 * ========================================================== */
        #define vPortSVCHandler         SVC_Handler
        #define xPortPendSVHandler      PendSV_Handler
    #define xPortSysTickHandler     SysTick_Handler
#endif /* FREERTOS_CONFIG_H - AGORA ESTÁ FECHADO CORRETAMENTE */