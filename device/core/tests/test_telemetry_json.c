#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "tk_telemetry_json.h"

static tk_telemetry_t make_telemetry(
    tk_mains_power_state_t mains_power,
    tk_pump_relay_state_t pump_relay,
    uint64_t unix_time_ms,
    uint32_t seq
)
{
    const tk_telemetry_t telemetry = {
        .input_state = {
            .mains_power = mains_power,
            .pump_relay = pump_relay,
        },
        .wall_time_state = TK_WALL_TIME_UNSYNCED,
        .unix_time_ms = unix_time_ms,
        .seq = seq,
    };

    return telemetry;
}

static void test_serializes_telemetry(void)
{
    const tk_telemetry_t telemetry = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_INACTIVE,
        123456789U,
        42U
    );

    const char *expected =
        "{\"meta\":{\"unix_time_ms\":123456789,\"seq\":42},"
        "\"payload\":{\"readings\":{"
        "\"mains_power\":\"present\","
        "\"pump_relay\":\"inactive\"},"
        "\"faults\":[]}}";

    char output[256];

    const int length = tk_telemetry_json_serialize(
        &telemetry,
        output,
        sizeof(output)
    );

    assert(length >= 0);
    assert((size_t)length == strlen(expected));
    assert(strcmp(output, expected) == 0);
}

static void test_serializes_fault_states(void)
{
    const tk_telemetry_t telemetry = make_telemetry(
        TK_MAINS_POWER_FAULT,
        TK_PUMP_RELAY_FAULT,
        0U,
        0U
    );

    const char *expected =
        "{\"meta\":{\"unix_time_ms\":0,\"seq\":0},"
        "\"payload\":{\"readings\":{"
        "\"mains_power\":\"fault\","
        "\"pump_relay\":\"fault\"},"
        "\"faults\":[]}}";

    char output[256];

    const int length = tk_telemetry_json_serialize(
        &telemetry,
        output,
        sizeof(output)
    );

    assert(length >= 0);
    assert(strcmp(output, expected) == 0);
}

static void test_rejects_invalid_arguments(void)
{
    const tk_telemetry_t telemetry = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_ACTIVE,
        0U,
        0U
    );

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

static void test_rejects_truncated_output(void)
{
    const tk_telemetry_t telemetry = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_ACTIVE,
        0U,
        0U
    );

    char output[8];

    assert(
        tk_telemetry_json_serialize(
            &telemetry,
            output,
            sizeof(output)
        ) == -1
    );
}

static void test_rejects_invalid_states(void)
{
    tk_telemetry_t telemetry = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_ACTIVE,
        0U,
        0U
    );

    char output[256];

    telemetry.input_state.mains_power =
        (tk_mains_power_state_t)99;

    assert(
        tk_telemetry_json_serialize(
            &telemetry,
            output,
            sizeof(output)
        ) == -1
    );

    telemetry.input_state.mains_power =
        TK_MAINS_POWER_PRESENT;

    telemetry.input_state.pump_relay =
        (tk_pump_relay_state_t)99;

    assert(
        tk_telemetry_json_serialize(
            &telemetry,
            output,
            sizeof(output)
        ) == -1
    );
}

static void test_accepts_exact_output_capacity(void)
{
    const tk_telemetry_t telemetry = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_ACTIVE,
        0U,
        0U
    );

    static const char expected[] =
        "{\"meta\":{\"unix_time_ms\":0,\"seq\":0},"
        "\"payload\":{\"readings\":{"
        "\"mains_power\":\"present\","
        "\"pump_relay\":\"active\"},"
        "\"faults\":[]}}";

    char output[sizeof(expected)];

    const int length = tk_telemetry_json_serialize(
        &telemetry,
        output,
        sizeof(output)
    );

    assert(
        length ==
        (int)(sizeof(expected) - 1U)
    );

    assert(strcmp(output, expected) == 0);
}

int main(void)
{
    test_serializes_telemetry();
    test_serializes_fault_states();
    test_rejects_invalid_arguments();
    test_rejects_truncated_output();
    test_rejects_invalid_states();
    test_accepts_exact_output_capacity();

    return 0;
}