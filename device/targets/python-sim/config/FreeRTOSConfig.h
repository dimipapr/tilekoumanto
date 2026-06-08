#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

extern uint32_t SystemCoreClock;

/* Target-owned values */
#define configCPU_CLOCK_HZ       SystemCoreClock
#define configTICK_RATE_HZ       1000
#define configMAX_PRIORITIES     5
#define configMINIMAL_STACK_SIZE 128
#define configMAX_TASK_NAME_LEN  16
#define configUSE_16_BIT_TICKS   0
#define configIDLE_SHOULD_YIELD  1

/* STM32/Cortex-M port-owned values */
#define configPRIO_BITS          4

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#include "FreeRTOSConfig_core.h"

#endif