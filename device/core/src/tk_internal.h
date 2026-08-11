//device/core/include/tk_internal.h

#ifndef TK_INTERNAL_H
#define TK_INTERNAL_H

#include "tk_platform.h"

typedef struct{
    const tk_platform_t *platform;
}tk_log_task_context_t;

typedef struct{
    const tk_platform_t *platform;
}tk_telemetry_task_context_t;

typedef struct{
    const tk_platform_t *platform;
}tk_status_task_context_t;

int tk_core_stop_requested(void);

int tk_log_runtime_init(void);
void tk_log_task(void *argument);
void tk_telemetry_task(void *argument);
void tk_status_task(void *argument);

uint64_t tk_runtime_ms(void);

#endif