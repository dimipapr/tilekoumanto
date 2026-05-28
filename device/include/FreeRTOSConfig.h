#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      ( ( unsigned long ) 1000000 )
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 128 )
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_MUTEXES                       1
#define configUSE_TIMERS                        0

/* Strict Static Allocation Only */
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        0

/* Hook Functions */
#define configUSE_IDLE_HOOK                     1
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            0
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

/* API Inclusion Options */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskCleanUpResources           0
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetSchedulerState          1

#if defined(PLATFORM_STM32)
    /* Cortex-M specific definitions. */
    #define configPRIO_BITS                       4        /* STM32F4xx uses 4 priority bits */

    /* The lowest interrupt priority that can be used in a call to a "set priority" function. */
    #define configLIBRARY_LOWEST_INTERRUPT_PRIORITY   15

    /* The highest interrupt priority that can be used by any interrupt service
    routine that makes call to interrupt safe FreeRTOS API functions. */
    #define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

    /* Memory map values transformed into the format needed by the nested vector interrupt controller. */
    #define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
    #define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#endif

#endif /* FREERTOS_CONFIG_H */