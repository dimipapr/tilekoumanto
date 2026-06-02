// device/core/tests/test_logic.c

#include "tk_telemetry.h"
#include "tk_types.h"

#include <stdio.h>

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
    tk_telemetry_t telemetry;

    telemetry.mains_power = mains_power;
    telemetry.pump_relay = pump_relay;
    telemetry.unix_time_ms = 0;

    return telemetry;
}

int main(void)
{
    tk_telemetry_t last;
    tk_telemetry_t current;

    last = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_INACTIVE
    );

    current = make_telemetry(
        TK_MAINS_POWER_PRESENT,
        TK_PUMP_RELAY_INACTIVE
    );

    expect_int(
        "publishes when there is no last sample",
        tk_should_publish_telemetry(0, &current, 0),
        1
    );

    expect_int(
        "does not publish unchanged sample before timeout",
        tk_should_publish_telemetry(&last, &current, 1000),
        0
    );

    if (g_failures != 0) {
        return 1;
    }

    return 0;
}