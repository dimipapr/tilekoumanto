// device/core/src/tk_core.c

#include "tk_core.h"
#include "tk_internal.h"
#include "tk_log.h"
#include "tk_telemetry.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>

#define TK_LOG_TASK_STACK_WORDS 256U
#define TK_TELEMETRY_TASK_STACK_WORDS 512
#define TK_STATUS_TASK_STACK_WORDS 128
#define TK_IDLE_TASK_STACK_WORDS configMINIMAL_STACK_SIZE

static tk_log_task_context_t g_log_task_context;
static tk_telemetry_task_context_t g_telemetry_task_context;
static tk_status_task_context_t g_status_task_context;

static const tk_platform_t *g_platform = 0;

static StaticTask_t g_idle_task_tcb;
static StackType_t g_idle_task_stack[TK_IDLE_TASK_STACK_WORDS];

static StaticTask_t g_log_task_tcb;
static StackType_t g_log_task_stack[TK_LOG_TASK_STACK_WORDS];

static StaticTask_t g_telemetry_task_tcb;
static StackType_t g_telemetry_task_stack[TK_TELEMETRY_TASK_STACK_WORDS];

static StaticTask_t g_status_task_tcb;
static StackType_t g_status_task_stack[TK_STATUS_TASK_STACK_WORDS];

static int tk_core_setup(const tk_platform_t *platform);
static int tk_core_create_tasks(void);



int tk_core_stop_requested(void){
    if (g_platform == 0 || g_platform->should_stop == 0)return 0;
    return g_platform->should_stop() != 0;
}

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

int tk_core_version(void)
{
    return 1;
}

static int tk_core_setup(const tk_platform_t *platform){
    if (platform == 0)                      return -1;
    if (platform->log == 0)                 return -1;
    if (platform->get_unix_time_ms == 0)        return -1;
    if (platform->read_inputs == 0)      return -1;
    if (platform->publish_telemetry == 0)   return -1;

    g_platform = platform;

    if (tk_log_runtime_init() != 0){
        (void)tk_log_sync(
            platform,
            "core log queue creation failed"
        );
        return -1;
    }

    (void)tk_log_sync(platform, "core starting");

    return 0;
}

int tk_core_run(const tk_platform_t *platform)
{

    if (tk_core_setup(platform) != 0)return -1;
    (void)tk_log_sync(platform, "core creating tasks");

    if (tk_core_create_tasks() != 0) return -1;
    (void)tk_log_sync(platform, "core starting scheduler");

    vTaskStartScheduler();

    (void)tk_log_sync(platform, "core scheduler stopped unexpectedly");

    return -1;
}

static int g_runtime_initialized = 0;
static TickType_t g_last_runtime_tick = 0;
static uint64_t g_runtime_ticks = 0U;

uint64_t tk_runtime_ms(void)
{
    const TickType_t current_tick = xTaskGetTickCount();

    if (g_runtime_initialized == 0) {
        g_runtime_ticks = (uint64_t)current_tick;
        g_runtime_initialized = 1;
    } else {
        g_runtime_ticks += (uint64_t)(TickType_t)(
            current_tick - g_last_runtime_tick
        );
    }

    g_last_runtime_tick = current_tick;

    return g_runtime_ticks * (uint64_t)portTICK_PERIOD_MS;
}

static int tk_core_create_tasks(void)
{
    TaskHandle_t log_task;
    TaskHandle_t status_task;
    TaskHandle_t telemetry_task;

    g_log_task_context.platform = g_platform;

    log_task = xTaskCreateStatic(
        tk_log_task,
        "logger",
        TK_LOG_TASK_STACK_WORDS,
        &g_log_task_context,
        tskIDLE_PRIORITY,
        g_log_task_stack,
        &g_log_task_tcb
    );

    if (log_task == 0){
        (void)tk_log_sync(
            g_platform,
            "logger task creation failed"
        );
        return -1;
    }

    (void)tk_log_sync(
        g_platform,
        "logger task created"
    );

    g_telemetry_task_context.platform = g_platform;
    
    telemetry_task = xTaskCreateStatic(
        tk_telemetry_task,
        "telemetry",
        TK_TELEMETRY_TASK_STACK_WORDS,
        &g_telemetry_task_context,
        tskIDLE_PRIORITY + 1U,
        g_telemetry_task_stack,
        &g_telemetry_task_tcb
    );


    if (telemetry_task == 0) {
        (void)tk_log_sync(g_platform, "telemetry task create failed");
        return -1;
    }

    (void)tk_log_sync(g_platform, "telemetry task created");

    g_status_task_context.platform = g_platform;

    status_task = xTaskCreateStatic(
        tk_status_task,
        "status",
        TK_STATUS_TASK_STACK_WORDS,
        &g_status_task_context,
        tskIDLE_PRIORITY + 1U,
        g_status_task_stack,
        &g_status_task_tcb
    );

    if (status_task == 0) {
        (void)tk_log_sync(g_platform, "status task create failed");
        return -1;
    }

    (void)tk_log_sync(g_platform, "status task created");

    return 0;
}