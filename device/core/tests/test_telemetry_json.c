#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "tk_telemetry_json.h"

static void test_serializes_telemetry(void){
    const tk_telemetry_t telemetry = {
        .mains_power = TK_MAINS_POWER_PRESENT,
        .pump_relay = TK_PUMP_RELAY_INACTIVE,
        .unix_time_ms = 123456789U,
        .seq = 42U,
    };

    const char *expected =
        "{\"meta\":{\"unix_time_ms\":123456789,\"seq\":42},"
        "\"payload\":{\"readings\":{"
        "\"mains_power\":\"present\","
        "\"pump_relay\":\"inactive\"},"
        "\"faults\":[]}}";

    char output[256];

    int length = tk_telemetry_json_serialize(
        &telemetry,
        output,
        sizeof(output)
    );

    assert(length >= 0);
    assert((size_t)length == strlen(expected));
    assert(strcmp(output, expected) == 0);
}

static void test_serializes_fault_states(void){
    const tk_telemetry_t telemetry = {
        .mains_power = TK_MAINS_POWER_FAULT,
        .pump_relay = TK_PUMP_RELAY_FAULT,
        .unix_time_ms = 0U,
        .seq = 0U,
    };

    const char *expected =
        "{\"meta\":{\"unix_time_ms\":0,\"seq\":0},"
        "\"payload\":{\"readings\":{"
        "\"mains_power\":\"fault\","
        "\"pump_relay\":\"fault\"},"
        "\"faults\":[]}}";

    char output[256];

    int length = tk_telemetry_json_serialize(
        &telemetry,
        output,
        sizeof(output)
    );

    assert(length >= 0);
    assert(strcmp(output, expected) == 0);
}

static void test_rejects_invalid_arguments(void){
    const tk_telemetry_t telemetry = {
        .mains_power = TK_MAINS_POWER_PRESENT,
        .pump_relay = TK_PUMP_RELAY_ACTIVE,
        .unix_time_ms = 0U,
        .seq = 0U,
    };

    char output[256];

    assert(
        tk_telemetry_json_serialize(
            NULL,
            output,
            sizeof(output)
        ) == -1
    );

    assert(
        tk_telemetry_json_serialize(
            &telemetry,
            NULL,
            sizeof(output)
        ) == -1
    );

    assert(
        tk_telemetry_json_serialize(
            &telemetry,
            output,
            0U
        ) == -1
    );
}

static void test_rejects_truncated_output(void){
    const tk_telemetry_t telemetry = {
        .mains_power = TK_MAINS_POWER_PRESENT,
        .pump_relay = TK_PUMP_RELAY_ACTIVE,
        .unix_time_ms = 0U,
        .seq = 0U,
    };

    char output[8];

    assert(
        tk_telemetry_json_serialize(
            &telemetry,
            output,
            sizeof(output)
        ) == -1
    );
}

int main(void){
    test_serializes_telemetry();
    test_serializes_fault_states();
    test_rejects_invalid_arguments();
    test_rejects_truncated_output();

    return 0;
}