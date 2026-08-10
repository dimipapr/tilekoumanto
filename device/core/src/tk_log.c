#include "tk_log.h"
#include "tk_internal.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#define TK_LOG_MESSAGE_SIZE 128U
#define TK_LOG_QUEUE_LENGTH 16U

typedef struct {
    char message[TK_LOG_MESSAGE_SIZE];
} tk_log_record_t;

static StaticQueue_t g_log_queue_control;

static uint8_t g_log_queue_storage[
    TK_LOG_QUEUE_LENGTH * sizeof(tk_log_record_t)
];

static QueueHandle_t g_log_queue = 0;
static uint32_t g_dropped_log_count = 0U;

static int tk_log_format(
    tk_log_record_t *record,
    const char *format,
    va_list arguments
);

static void tk_log_record_dropped(void);
static uint32_t tk_log_take_dropped_count(void);

int tk_log_runtime_init(void){
    g_dropped_log_count = 0U;
    g_log_queue = xQueueCreateStatic(
        TK_LOG_QUEUE_LENGTH,
        sizeof(tk_log_record_t),
        g_log_queue_storage,
        &g_log_queue_control
    );
    return g_log_queue == 0? -1 : 0;
}

int tk_log_sync(
    const tk_platform_t *platform,
    const char *format,
    ...
){
    tk_log_record_t record;
    va_list arguments;
    int result;

    if(
        platform == 0 ||
        platform->log == 0 ||
        format == 0
    ){
        return -1;
    }

    va_start(arguments, format);
    result = tk_log_format(&record, format, arguments);
    va_end(arguments);

    if (result != 0){
        return -1;
    }
    platform->log(record.message);
    return 0;
}

int tk_log_enqueue(
    const char *format,
    ...
){
    tk_log_record_t record;
    va_list arguments;
    int result;

    if (g_log_queue == 0 || format == 0){
        return -1;
    }

    va_start(arguments, format);
    result = tk_log_format(&record, format, arguments);
    va_end(arguments);

    if (result != 0){
        return -1;
    }

    if (xQueueSend(g_log_queue, &record, 0) != pdPASS){
        tk_log_record_dropped();
        return -1;
    }
    return 0;
}

void tk_log_task(void *argument){
    const tk_log_task_context_t *context = argument;
    const tk_platform_t *platform;
    tk_log_record_t record;

    if (context == 0 || 
    context->platform == 0 ||
    context->platform->log == 0 ||
    g_log_queue == 0
    ){
        for(;;){
            vTaskDelay(portMAX_DELAY);
        }
    }
    platform = context->platform;

    for(;;){
        uint32_t dropped_count;
        if(xQueueReceive(
            g_log_queue,
            &record,
            portMAX_DELAY
        ) != pdPASS){
            continue;
        }platform->log(record.message);
        dropped_count = tk_log_take_dropped_count();
        if (dropped_count != 0U){
            char dropped_message[TK_LOG_MESSAGE_SIZE];
            int written;

            written = snprintf(
                dropped_message,
                sizeof(dropped_message),
                "logger dropped %" PRIu32 " messages",
                dropped_count
            );
            if (written >= 0 ){
                dropped_message[sizeof(dropped_message) - 1U] = '\0';
                platform->log(dropped_message);
            }
        }
    }
}

static int tk_log_format(
    tk_log_record_t *record,
    const char *format,
    va_list arguments
){
    int written;
    if (record == 0 || format == 0){
        return -1;
    }
    written = vsnprintf(
        record->message,
        sizeof(record->message),
        format,
        arguments
    );

    if (written < 0)return -1;
    record->message[sizeof(record->message) - 1U] = '\0';
    return 0;
}

static void tk_log_record_dropped(void){
    taskENTER_CRITICAL();
    if (g_dropped_log_count < UINT32_MAX){
        g_dropped_log_count ++;
    }
    taskEXIT_CRITICAL();
}

static uint32_t tk_log_take_dropped_count(void){
    uint32_t dropped_count;
    taskENTER_CRITICAL();
    dropped_count = g_dropped_log_count;
    g_dropped_log_count = 0U;
    taskEXIT_CRITICAL();

    return dropped_count;
}