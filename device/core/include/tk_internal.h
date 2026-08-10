//device/core/include/tk_internal.h

#ifndef TK_INTERNAL_H
#define TK_INTERNAL_H

#include "tk_platform.h"

typedef struct
{
    const tk_platform_t *platform;
}tk_log_task_context_t;

int tk_core_stop_requested(void);

void tk_log_task(void *argument);

int tk_log_runtime_init(void);

#endif