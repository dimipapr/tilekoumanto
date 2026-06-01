// device/core/src/tk_telemetry.c

#include "tk_telemetry.h"
#include "tk_internal.h"
#include "tk_platform.h"
#include "tk_types.h"

#include "FreeRTOS.h"
#include "task.h"

#include <inttypes.h>
#include <stdint.h>

#define TK_PUBLISH_TIMEOUT_MS 30000ULL
#define TK_TELEMETRY_TASK_PERIOD_MS 1000U

static int g_has_last_published = 0;
static tk_telemetry_t g_last_published;
static TickType_t g_last_publish_tick = 0;

static const char *tk_mains_power_to_string(tk_mains_power_state_t state);
static const char *tk_pump_relay_to_string(tk_pump_relay_state_t state);

static int tk_process_telemetry_once(const tk_platform_t *platform);
static uint64_t tk_elapsed_ticks_to_ms(TickType_t start_tick,TickType_t end_tick);

static uint64_t tk_elapsed_ticks_to_ms(
    TickType_t start_tick,
    TickType_t end_tick
)
{
    TickType_t elapsed_ticks;

    elapsed_ticks = end_tick - start_tick;

    return (uint64_t)elapsed_ticks * (uint64_t)portTICK_PERIOD_MS;
}

static const char *tk_mains_power_to_string(tk_mains_power_state_t state)
{
    switch (state) {
    case TK_MAINS_POWER_PRESENT:
        return "present";
    case TK_MAINS_POWER_NOT_PRESENT:
        return "not_present";
    case TK_MAINS_POWER_FAULT:
        return "fault";
    default:
        return "unknown";
    }
}

static const char *tk_pump_relay_to_string(tk_pump_relay_state_t state)
{
    switch (state) {
    case TK_PUMP_RELAY_ACTIVE:
        return "active";
    case TK_PUMP_RELAY_INACTIVE:
        return "inactive";
    case TK_PUMP_RELAY_FAULT:
        return "fault";
    default:
        return "unknown";
    }
}

int tk_should_publish_telemetry(
    const tk_telemetry_t *last_published,
    const tk_telemetry_t *current,
    uint64_t time_since_last_publish_ms
){
    if (current == 0) {return 0;}

    if (last_published == 0) {return 1;}

    if (last_published->mains_power != current->mains_power) {return 1;}

    if (last_published->pump_relay != current->pump_relay) {return 1;}

    if (time_since_last_publish_ms >= TK_PUBLISH_TIMEOUT_MS) {
        return 1;
    }

    return 0;
}

static int tk_process_telemetry_once(const tk_platform_t *platform)
{
    TickType_t current_tick;
    uint64_t time_since_last_publish_ms;
    tk_telemetry_t telemetry = {0};

    if (platform == 0) {
        return 0;
    }

    if (platform->read_telemetry == 0) {
        tk_log(platform, "read_telemetry callback missing");
        return 0;
    }

    if (platform->read_telemetry(&telemetry) == 0) {
        tk_log(platform, "read_telemetry failed");
        return 0;
    }

    tk_log(
        platform,
        "mains:%s relay:%s time:%" PRIu64,
        tk_mains_power_to_string(telemetry.mains_power),
        tk_pump_relay_to_string(telemetry.pump_relay),
        telemetry.unix_time_ms
    );

    current_tick = xTaskGetTickCount();

    if (g_has_last_published) {
        time_since_last_publish_ms = tk_elapsed_ticks_to_ms(
            g_last_publish_tick,
            current_tick
        );
    } else {
        time_since_last_publish_ms = 0;
    }

    if (!tk_should_publish_telemetry(
            g_has_last_published ? &g_last_published : 0,
            &telemetry,
            time_since_last_publish_ms
        )) {
        tk_log(platform, "publish_telemetry skipped");
        return 1;
    }

    if (platform->publish_telemetry == 0) {
        tk_log(platform, "publish_telemetry callback missing");
        return 0;
    }

    if (platform->publish_telemetry(&telemetry) == 0) {
        tk_log(platform, "publish_telemetry failed");
        return 0;
    }

    g_last_published = telemetry;
    g_last_publish_tick = current_tick;
    g_has_last_published = 1;

    tk_log(platform, "publish_telemetry complete");

    return 1;
}

void tk_telemetry_task(void *argument)
{
    const tk_telemetry_task_context_t *context = argument;
    const tk_platform_t *platform;
    TickType_t last_wake_tick;

    if (context == 0 || context->platform == 0){
        vTaskEndScheduler();
        for (;;) {}
    }

    platform = context->platform;

    tk_log(platform, "telemetry task started");

    last_wake_tick = xTaskGetTickCount();

    for (;;) {
        if (tk_core_stop_requested()) {
            tk_log(platform, "telemetry task stop requested");
            break;
        }

        if (!tk_process_telemetry_once(platform)) {
            tk_log(platform, "telemetry processing failed");
        }

        vTaskDelayUntil(&last_wake_tick, pdMS_TO_TICKS(TK_TELEMETRY_TASK_PERIOD_MS));
    }

    tk_log(platform, "telemetry task complete");
    vTaskEndScheduler();

    for (;;) {
    }
}