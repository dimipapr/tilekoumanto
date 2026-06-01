// device/core/include/tk_telemetry.h

#ifndef TK_TELEMETRY_H
#define TK_TELEMETRY_H

#include "tk_platform.h"

typedef struct {
    const tk_platform_t *platform;
} tk_telemetry_task_context_t;


void tk_telemetry_task(void *argument);

int tk_should_publish_telemetry(
    const tk_telemetry_t *last_published,
    const tk_telemetry_t *current
);

#endif