// device/core/src/tk_telemetry.c

#include "tk_telemetry.h"
#include "tk_types.h"

#include <stdint.h>

#define TK_PUBLISH_TIMEOUT_MS 30000ULL

int tk_should_publish_telemetry(
    const tk_telemetry_t *last_published,
    const tk_telemetry_t *current,
    uint64_t time_since_last_publish_ms
)
{
    if (current == 0) {
        return 0;
    }

    if (last_published == 0) {
        return 1;
    }

    if (last_published->mains_power != current->mains_power) {
        return 1;
    }

    if (last_published->pump_relay != current->pump_relay) {
        return 1;
    }

    if (time_since_last_publish_ms >= TK_PUBLISH_TIMEOUT_MS) {
        return 1;
    }

    return 0;
}