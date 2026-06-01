#include "tk_core.h"

int tk_core_version(void){
    return 1;
}

int tk_should_publish_telemetry(const tk_telemetry_t *last_published, const tk_telemetry_t *current){
    if (current == 0)                                           return 0;
    if (last_published == 0)                                    return 1;
    if (last_published->mains_power != current->mains_power)    return 1;
    if (last_published->pump_relay != current->pump_relay)      return 1;

    return 0;
}