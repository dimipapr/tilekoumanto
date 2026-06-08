// device/core/src/tk_core.c

#include "tk_core.h"
#include "tk_internal.h"
#include "tk_telemetry.h"
#include "tk_status.h"

#include "FreeRTOS.h"
#include "task.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#define TK_LOG_BUFFER_SIZE 128

#define TK_TELEMETRY_TASK_STACK_WORDS 512
#define TK_STATUS_TASK_STACK_WORDS 128
#define TK_IDLE_TASK_STACK_WORDS configMINIMAL_STACK_SIZE

static tk_telemetry_task_context_t g_telemetry_task_context;
static tk_status_task_context_t g_status_task_context;

static const tk_platform_t *g_platform = 0;

static StaticTask_t g_idle_task_tcb;
static StackType_t g_idle_task_stack[TK_IDLE_TASK_STACK_WORDS];

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

void tk_log(const tk_platform_t *platform, const char *format, ...)
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

int tk_core_version(void)
{
    return 1;
}

int tk_core_run(const tk_platform_t *platform)
{

    if (tk_core_setup(platform) != 0)return -1;
    tk_log(platform, "core creating tasks");

    if (tk_core_create_tasks() != 0) return -1;
    tk_log(platform, "core starting scheduler");

    vTaskStartScheduler();

    tk_log(platform, "core scheduler stopped unexpectedly");

    return -1;
}

static int tk_core_setup(const tk_platform_t *platform){
    if (platform == 0)                      return -1;
    if (platform->log == 0)                 return -1;
    if (platform->unix_time_ms == 0)        return -1;
    if (platform->read_telemetry == 0)      return -1;
    if (platform->publish_telemetry == 0)   return -1;

    g_platform = platform;

    tk_log(platform, "core starting");
    tk_log(platform, "core platform time ms: %" PRIu64, platform->unix_time_ms());

    return 0;
}

static int tk_core_create_tasks(void)
{
    TaskHandle_t telemetry_task;

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
        tk_log(g_platform, "telemetry task create failed");
        return -1;
    }

    tk_log(g_platform, "telemetry task created");

    TaskHandle_t status_task;

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
        tk_log(g_platform, "status task create failed");
        return -1;
    }

    tk_log(g_platform, "status task created");

    return 0;
}

uint64_t tk_core_runtime_ms(void){
    return (uint64_t)xTaskGetTickCount() * (uint64_t)portTICK_PERIOD_MS;
}