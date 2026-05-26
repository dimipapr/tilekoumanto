#ifndef TK_HAL_H
#define TK_HAL_H

#include <stdbool.h>
#include <stdint.h>

// 1. Telemetry Data Structure
typedef struct {
    bool mains_present;
    bool relay_active;
} tk_telemetry_t;

// 2. Environment Callbacks (Injected by STM32 or Python)
typedef void (*hal_get_telemetry_cb)(tk_telemetry_t *telemetry);
typedef bool (*hal_mqtt_publish_cb)(const char *topic, const char *payload);
typedef uint64_t (*hal_get_unix_time_cb)(void);

// 3. Concurrency Callbacks (For RTOS/FFI thread safety)
typedef void (*hal_lock_cb)(void);
typedef void (*hal_unlock_cb)(void);

// 4. Core Logic API
void tk_core_init(
    const char *_uuid,
    hal_get_telemetry_cb tel_cb,
    hal_mqtt_publish_cb pub_cb,
    hal_get_unix_time_cb time_cb,
    hal_lock_cb lock_cb,
    hal_unlock_cb unlock_cb
);

void tk_core_tick(void);

#endif // TK_HAL_H