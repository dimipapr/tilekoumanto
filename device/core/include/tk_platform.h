// device/core/include/tk_platform.h

#ifndef TK_PLATFORM_H
#define TK_PLATFORM_H

#include <stdint.h>

#include "tk_types.h"

typedef struct {
    void (*log)(const char *message);
    uint64_t (*unix_time_ms)(void);

    int (*read_telemetry)(tk_telemetry_t *out);
    int (*publish_telemetry)(const tk_telemetry_t *tk_telemetry);
    int (*should_stop)(void);
} tk_platform_t;

#endif