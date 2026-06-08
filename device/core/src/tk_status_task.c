// device/core/src/tk_status_task.c

#include "tk_status.h"
#include "tk_internal.h"

#include "FreeRTOS.h"
#include "task.h"

#define TK_STATUS_TASK_PERIOD_MS 500U

void tk_status_task(void *argument)
{
    const tk_status_task_context_t *context = argument;
    const tk_platform_t *platform;

    if (context == 0 || context->platform == 0) {
        vTaskEndScheduler();
        for (;;) {
        }
    }

    platform = context->platform;

    tk_log(platform, "status task started");

    for (;;) {
        if (tk_core_stop_requested()) {
            tk_log(platform, "status task stop requested");
            break;
        }

        if (platform->status_led_toggle != 0) {
            if (platform->status_led_toggle() != 0) {
                tk_log(platform, "status led toggle failed");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(TK_STATUS_TASK_PERIOD_MS));
    }

    tk_log(platform, "status task complete");
    vTaskEndScheduler();

    for (;;) {
    }
}