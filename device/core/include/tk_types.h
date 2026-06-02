// device/core/include/tk_types.h

#ifndef TK_TYPES_H
#define TK_TYPES_H

#include <stdint.h>

typedef enum {
    TK_MAINS_POWER_PRESENT = 0,
    TK_MAINS_POWER_NOT_PRESENT = 1,
    TK_MAINS_POWER_FAULT = 2
} tk_mains_power_state_t;

typedef enum {
    TK_PUMP_RELAY_ACTIVE = 0,
    TK_PUMP_RELAY_INACTIVE = 1,
    TK_PUMP_RELAY_FAULT = 2
} tk_pump_relay_state_t;

typedef struct {
    tk_mains_power_state_t mains_power;
    tk_pump_relay_state_t pump_relay;
    uint64_t unix_time_ms;
    uint32_t seq;
} tk_telemetry_t;

#endif