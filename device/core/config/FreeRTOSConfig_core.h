// device/core/config/FreeRTOSConfig_core.h

#ifndef FREERTOS_CONFIG_CORE_H
#define FREERTOS_CONFIG_CORE_H

#ifdef configSUPPORT_STATIC_ALLOCATION
#error "configSUPPORT_STATIC_ALLOCATION is core-owned; do not define it in target FreeRTOSConfig.h"
#endif
#define configSUPPORT_STATIC_ALLOCATION 1

#ifdef configSUPPORT_DYNAMIC_ALLOCATION
#error "configSUPPORT_DYNAMIC_ALLOCATION is core-owned; do not define it in target FreeRTOSConfig.h"
#endif
#define configSUPPORT_DYNAMIC_ALLOCATION 0

#ifdef configUSE_IDLE_HOOK
#error "configUSE_IDLE_HOOK is core-owned; do not define it in target FreeRTOSConfig.h"
#endif
#define configUSE_IDLE_HOOK 1

#ifdef configCHECK_FOR_STACK_OVERFLOW
#error "configCHECK_FOR_STACK_OVERFLOW is core-owned; do not define it in target FreeRTOSConfig.h"
#endif
#define configCHECK_FOR_STACK_OVERFLOW 2

#ifdef INCLUDE_vTaskDelayUntil
#error "INCLUDE_vTaskDelayUntil is core-owned; do not define it in target FreeRTOSConfig.h"
#endif

#ifdef configUSE_PORT_OPTIMISED_TASK_SELECTION
#error "configUSE_PORT_OPTIMISED_TASK_SELECTION is core-owned; do not define it in target FreeRTOSConfig.h"
#endif
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0

#ifdef configUSE_APPLICATION_TASK_TAG
#error "configUSE_APPLICATION_TASK_TAG is core-owned; do not define it in target FreeRTOSConfig.h"
#endif
#define configUSE_APPLICATION_TASK_TAG 0

#define INCLUDE_vTaskDelayUntil 1

#define configUSE_PREEMPTION                     1
#define configUSE_TICK_HOOK                      0
#define configUSE_TIMERS                         0
#define configUSE_MUTEXES                        1
#define configUSE_RECURSIVE_MUTEXES              0
#define configUSE_COUNTING_SEMAPHORES            0
#define configQUEUE_REGISTRY_SIZE                0

#define INCLUDE_vTaskDelay                       1
#define INCLUDE_xTaskGetSchedulerState           1
#define INCLUDE_xTaskGetCurrentTaskHandle        1

#endif