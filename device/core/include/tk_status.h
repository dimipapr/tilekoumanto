// device/core/include/tk_status.h

#ifndef TK_STATUS_H
#define TK_STATUS_H

#include "tk_platform.h"

typedef struct {
    const tk_platform_t *platform;
} tk_status_task_context_t;

void tk_status_task(void *argument);

#endif