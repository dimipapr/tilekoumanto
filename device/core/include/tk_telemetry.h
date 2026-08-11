// device/core/include/tk_telemetry.h

#ifndef TK_TELEMETRY_H
#define TK_TELEMETRY_H

#include <stdint.h>

#include "tk_types.h"

int tk_should_publish_telemetry(
    const tk_telemetry_t *last_published,
    const tk_telemetry_t *current,
    uint64_t time_since_last_publish_ms
);

#endif