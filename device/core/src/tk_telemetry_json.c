#include "tk_telemetry_json.h"

#include <inttypes.h>
#include <stdio.h>

static const char *mains_power_to_string(tk_mains_power_state_t state){
    switch (state){
        case TK_MAINS_POWER_PRESENT:
            return "present";
        case TK_MAINS_POWER_NOT_PRESENT:
            return "not_present";
        case TK_MAINS_POWER_FAULT:
            return "fault";
        default:
            return NULL;
    }
}

static const char *pump_relay_to_string(tk_pump_relay_state_t state){
    switch (state)
    {
    case TK_PUMP_RELAY_ACTIVE:
        return "active";
    case TK_PUMP_RELAY_INACTIVE:
        return "inactive";
    case TK_PUMP_RELAY_FAULT:
        return "fault";
    default:
        return NULL;
    }
}

int tk_telemetry_json_serialize(
    const tk_telemetry_t *telemetry,
    char *output,
    size_t output_capacity
){

    const char *mains_power;
    const char *pump_relay;
    int length;

    if (
        telemetry == NULL ||
        output == NULL ||
        output_capacity == 0U
    ){
        return -1;
    }

    mains_power = mains_power_to_string(telemetry->input_state.mains_power);
    pump_relay = pump_relay_to_string(telemetry->input_state.pump_relay);

    if (mains_power == NULL || pump_relay == NULL)return -1;

    length = snprintf(
        output,
        output_capacity,
        "{\"meta\":{\"unix_time_ms\":%" PRIu64
        ",\"seq\":%" PRIu32 "},"
        "\"payload\":{\"readings\":{"
        "\"mains_power\":\"%s\","
        "\"pump_relay\":\"%s\"},"
        "\"faults\":[]}}",
        telemetry->unix_time_ms,
        telemetry->seq,
        mains_power,
        pump_relay
    );

    if (length < 0 || (size_t)length >= output_capacity){
        return -1;
    }

    return length;
}