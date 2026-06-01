#include "tk_core.h"
#include "tk_internal.h"

#include "FreeRTOS.h"
#include "task.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#define TK_LOG_BUFFER_SIZE 128

#define TK_CORE_TASK_STACK_WORDS 512
#define TK_IDLE_TASK_STACK_WORDS configMINIMAL_STACK_SIZE

#define TK_CORE_SIM_LOOP_COUNT 5U
#define TK_CORE_LOOP_PERIOD_MS 1000U

#if configSUPPORT_STATIC_ALLOCATION != 1
#error "configSUPPORT_STATIC_ALLOCATION is not enabled"
#endif

static int g_has_last_published = 0;
static tk_telemetry_t g_last_published;

static const tk_platform_t *g_platform = 0;

static StaticTask_t g_idle_task_tcb;
static StackType_t g_idle_task_stack[TK_IDLE_TASK_STACK_WORDS];

static StaticTask_t g_core_task_tcb;
static StackType_t g_core_task_stack[TK_CORE_TASK_STACK_WORDS];

static void tk_log(const tk_platform_t *platform, const char *format, ...);
static void tk_core_task(void *argument);
static int tk_process_telemetry_once(const tk_platform_t *platform);

void vApplicationGetIdleTaskMemory(
    StaticTask_t **task_tcb,
    StackType_t **task_stack,
    StackType_t *task_stack_size
)
{
    *task_tcb = &g_idle_task_tcb;
    *task_stack = g_idle_task_stack;
    *task_stack_size = TK_IDLE_TASK_STACK_WORDS;
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    (void)task;
    (void)task_name;

    for (;;) {
    }
}

static void tk_log(const tk_platform_t *platform, const char *format, ...)
{
    char buffer[TK_LOG_BUFFER_SIZE];
    va_list args;
    int written;

    if (platform == 0 || platform->log == 0 || format == 0) {
        return;
    }

    va_start(args, format);
    written = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (written < 0) {
        platform->log("core log formatting failed");
        return;
    }

    buffer[sizeof(buffer) - 1U] = '\0';

    platform->log(buffer);
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
    const tk_telemetry_t *current
)
{
    uint64_t elapsed_time_ms;

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

    elapsed_time_ms = current->unix_time_ms - last_published->unix_time_ms;

    if (elapsed_time_ms >= TK_PUBLISH_TIMEOUT_MS) {
        return 1;
    }

    return 0;
}

static int tk_process_telemetry_once(const tk_platform_t *platform)
{
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

    if (!tk_should_publish_telemetry(
            g_has_last_published ? &g_last_published : 0,
            &telemetry
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
    g_has_last_published = 1;

    tk_log(platform, "publish_telemetry complete");

    return 1;
}

static void tk_core_task(void *argument)
{
    TickType_t last_wake_tick;
    unsigned int i;

    (void)argument;

    tk_log(g_platform, "core task started");

    last_wake_tick = xTaskGetTickCount();

    for (i = 0U; i < TK_CORE_SIM_LOOP_COUNT; i++) {
        tk_log(g_platform, "core loop %u", i);

        if (!tk_process_telemetry_once(g_platform)) {
            tk_log(g_platform, "core telemetry processing failed");
        }

        vTaskDelayUntil(&last_wake_tick, pdMS_TO_TICKS(TK_CORE_LOOP_PERIOD_MS));
    }

    tk_log(g_platform, "core task complete");

    /*
     * This lets the POSIX simulator return to Python after the test loop.
     * On the STM32 target, the real runtime should normally run forever.
     */
    vTaskEndScheduler();

    for (;;) {
    }
}

int tk_core_version(void)
{
    return 1;
}

int tk_core_run(const tk_platform_t *platform)
{
    TaskHandle_t task;

    if (platform == 0) {
        return 0;
    }

    if (platform->log == 0) {
        return 0;
    }

    if (platform->unix_time_ms == 0) {
        return 0;
    }

    g_platform = platform;

    tk_log(platform, "core starting");
    tk_log(platform, "core platform time ms: %" PRIu64, platform->unix_time_ms());

    task = xTaskCreateStatic(
        tk_core_task,
        "core",
        TK_CORE_TASK_STACK_WORDS,
        0,
        tskIDLE_PRIORITY + 1U,
        g_core_task_stack,
        &g_core_task_tcb
    );

    if (task == 0) {
        tk_log(platform, "core task create failed");
        return 0;
    }

    vTaskStartScheduler();

    tk_log(platform, "core scheduler stopped");

    return 1;
}