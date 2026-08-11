// device/core/include/tk_platform.h

#ifndef TK_PLATFORM_H
#define TK_PLATFORM_H

#include <stdint.h>

#include "tk_types.h"

typedef struct {
    void (*log)(const char *message);
    
    int (*read_inputs)(tk_input_state_t *out);

    tk_wall_time_state_t (*get_unix_time_ms)(
        uint64_t *out
    );
    
    int (*publish_telemetry)(
        const char *payload
    );
    
    int (*should_stop)(void);
    int (*status_led_toggle)(void);
} tk_platform_t;

#endif