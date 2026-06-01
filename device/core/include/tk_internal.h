#ifndef TK_INTERNAL_H
#define TK_INTERNAL_H

#include "tk_platform.h"
#include "tk_types.h"

typedef struct {
    int has_last_published;
    tk_telemetry_t last_published;
} tk_core_state_t;

int tk_core_step(
    tk_core_state_t *state,
    const tk_platform_t *platform
);

int tk_should_publish_telemetry_internal(
    const tk_telemetry_t *last_published,
    const tk_telemetry_t *current
);

#endif