// device/core/tests/test_logic.c

#include "tk_telemetry.h"

#include <stdio.h>

#define PUBLISH_TIMEOUT_MS 30000U

static int g_failures = 0;

static void expect_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        printf("FAIL: %s: expected %d, got %d\n", name, expected, actual);
        g_failures++;
        return;
    }

    printf("PASS: %s\n", name);
}

static tk_telemetry_t make_telemetry(
    tk_mains_power_state_t mains_power,
    tk_pump_relay_state_t pump_relay
)
{
    const tk_telemetry_t telemetry = {
        .input_state = {
            .mains_power = mains_power,
            .pump_relay = pump_relay,
        },
        .wall_time_state = TK_WALL_TIME_UNSYNCED,
        .unix_time_ms = 0U,
        .seq = 0U,
    };

    return telemetry;
}

static void test_rejects_missing_current_sample(void)
{
    const tk_telemetry_t last = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_INACTIVE
    );

    expect_int(
        "does not publish a missing current sample",
        tk_should_publish_telemetry(&last, NULL, 0U),
        0
    );
}

static void test_publishes_first_sample(void)
{
    const tk_telemetry_t current = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_INACTIVE
    );

    expect_int(
        "publishes when there is no last sample",
        tk_should_publish_telemetry(NULL, &current, 0U),
        1
    );
}

static void test_skips_unchanged_sample_before_timeout(void)
{
    const tk_telemetry_t last = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_INACTIVE
    );
    const tk_telemetry_t current = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_INACTIVE
    );

    expect_int(
        "does not publish unchanged sample before timeout",
        tk_should_publish_telemetry(
            &last,
            &current,
            PUBLISH_TIMEOUT_MS - 1U
        ),
        0
    );
}

static void test_publishes_mains_power_change(void)
{
    const tk_telemetry_t last = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_INACTIVE
    );
    const tk_telemetry_t current = make_telemetry(
        TK_MAINS_POWER_NOT_PRESENT,
        TK_PUMP_RELAY_INACTIVE
    );

    expect_int(
        "publishes when mains power changes",
        tk_should_publish_telemetry(&last, &current, 0U),
        1
    );
}

static void test_publishes_pump_relay_change(void)
{
    const tk_telemetry_t last = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_INACTIVE
    );
    const tk_telemetry_t current = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_ACTIVE
    );

    expect_int(
        "publishes when pump relay changes",
        tk_should_publish_telemetry(&last, &current, 0U),
        1
    );
}

static void test_publishes_at_timeout_boundary(void)
{
    const tk_telemetry_t last = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_INACTIVE
    );
    const tk_telemetry_t current = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_INACTIVE
    );

    expect_int(
        "publishes unchanged sample at timeout boundary",
        tk_should_publish_telemetry(
            &last,
            &current,
            PUBLISH_TIMEOUT_MS
        ),
        1
    );
}

static void test_ignores_message_metadata(void)
{
    tk_telemetry_t last = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_INACTIVE
    );
    tk_telemetry_t current = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_INACTIVE
    );

    last.unix_time_ms = 1000U;
    last.seq = 1U;
    current.unix_time_ms = 2000U;
    current.seq = 2U;

    expect_int(
        "does not publish for metadata-only changes",
        tk_should_publish_telemetry(&last, &current, 0U),
        0
    );
}

int main(void)
{
    test_rejects_missing_current_sample();
    test_publishes_first_sample();
    test_skips_unchanged_sample_before_timeout();
    test_publishes_mains_power_change();
    test_publishes_pump_relay_change();
    test_publishes_at_timeout_boundary();
    test_ignores_message_metadata();

    if (g_failures != 0) {
        return 1;
    }

    return 0;
}