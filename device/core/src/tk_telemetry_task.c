// device/core/src/tk_telemetry_task.c

#include "tk_internal.h"
#include "tk_log.h"
#include "tk_telemetry.h"
#include "tk_telemetry_json.h"

#include "FreeRTOS.h"
#include "task.h"

#include <inttypes.h>
#include <stdint.h>

#define TK_TELEMETRY_TASK_PERIOD_MS 1000U
#define TK_TELEMETRY_PAYLOAD_SIZE 256U

static int g_has_last_published = 0;
static tk_telemetry_t g_last_published;
static TickType_t g_last_publish_tick = 0;
static uint32_t g_next_telemetry_seq = 1U;

static int tk_process_telemetry_once(const tk_platform_t *platform);
static uint64_t tk_elapsed_ticks_to_ms(TickType_t start_tick, TickType_t end_tick);

static uint64_t tk_elapsed_ticks_to_ms(
    TickType_t start_tick,
    TickType_t end_tick
)
{
    const TickType_t elapsed_ticks = end_tick - start_tick;

    return (uint64_t)elapsed_ticks * (uint64_t)portTICK_PERIOD_MS;
}

static int tk_process_telemetry_once(const tk_platform_t *platform)
{
    char payload[TK_TELEMETRY_PAYLOAD_SIZE];
    int payload_length;
    tk_input_state_t input_state;
    tk_telemetry_t telemetry = {0};
    TickType_t current_tick;
    uint64_t time_since_last_publish_ms;

    if (platform == 0) {
        return -1;
    }

    if (platform->read_inputs == 0) {
        (void)tk_log_enqueue("read_inputs callback missing");
        return -1;
    }

    if (platform->read_inputs(&input_state) != 0) {
        (void)tk_log_enqueue( "read_inputs failed");
        return -1;
    }

    telemetry.input_state = input_state;
    telemetry.runtime_ms = tk_runtime_ms();

    if (platform->get_unix_time_ms == 0){
        (void)tk_log_enqueue(
            "get_unix_time_ms callback missing"
        );
        return -1;
    }

    telemetry.wall_time_state = 
        platform->get_unix_time_ms(
            &telemetry.unix_time_ms
        );

    switch(telemetry.wall_time_state){
        case TK_WALL_TIME_SYNCED:
            break;
        case TK_WALL_TIME_UNSYNCED:
        case TK_WALL_TIME_FAULT:
            telemetry.unix_time_ms = 0U;
            break;
        default:
            telemetry.wall_time_state = TK_WALL_TIME_FAULT;
            telemetry.unix_time_ms = 0U;
            break;
    }

    current_tick = xTaskGetTickCount();

    if (g_has_last_published) {
        time_since_last_publish_ms = tk_elapsed_ticks_to_ms(
            g_last_publish_tick,
            current_tick
        );
    } else {
        time_since_last_publish_ms = 0U;
    }

    if (!tk_should_publish_telemetry(
            g_has_last_published ? &g_last_published : 0,
            &telemetry,
            time_since_last_publish_ms
        )) {
        return 0;
    }
    
    telemetry.seq = g_next_telemetry_seq++;

    payload_length = tk_telemetry_json_serialize(
        &telemetry,
        payload,
        sizeof(payload)
    );

    if (payload_length < 0){
        (void)tk_log_enqueue("telemetry serialization failed");
        return -1;
    }

    if(platform->publish_telemetry == 0){
        (void)tk_log_enqueue(
            "publish telemetry callback missing"
        );
        return -1;
    }

    if (platform->publish_telemetry(payload) != 0){
        (void)tk_log_enqueue("publish_telemetry failed");
        return -1;
    }

    g_last_published = telemetry;
    g_last_publish_tick = current_tick;
    g_has_last_published = 1;

    (void)tk_log_enqueue(
        "publish_telemetry complete seq:%" PRIu32,
        telemetry.seq
    );

    return 0;
}

void tk_telemetry_task(void *argument)
{
    const tk_telemetry_task_context_t *context = argument;
    const tk_platform_t *platform;
    TickType_t last_wake_tick;

    if (context == 0 || context->platform == 0) {
        vTaskEndScheduler();
        for (;;) {
        }
    }

    platform = context->platform;

    (void)tk_log_enqueue("telemetry task started");

    last_wake_tick = xTaskGetTickCount();

    for (;;) {
        if (tk_core_stop_requested()) {
            (void)tk_log_enqueue("telemetry task stop requested");
            break;
        }

        (void)tk_process_telemetry_once(platform);

        vTaskDelayUntil(
            &last_wake_tick,
            pdMS_TO_TICKS(TK_TELEMETRY_TASK_PERIOD_MS)
        );
    }

    (void)tk_log_enqueue("telemetry task complete");
    vTaskEndScheduler();

    for (;;) {
    }
}