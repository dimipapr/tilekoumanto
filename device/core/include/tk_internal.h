#ifndef TK_INTERNAL_H
#define TK_INTERNAL_H

#include "tk_platform.h"
#include "tk_types.h"

#define TK_PUBLISH_TIMEOUT_MS 30000ULL

int tk_should_publish_telemetry(
    const tk_telemetry_t *last_published,
    const tk_telemetry_t *current
);

#endif