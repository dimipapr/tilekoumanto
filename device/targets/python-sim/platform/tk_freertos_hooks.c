// device/targets/python-sim/platform/tk_freertos_hooks.c

#include <time.h>

void vApplicationIdleHook(void)
{
    const struct timespec sleep_time = {
        .tv_sec = 0,
        .tv_nsec = 1000000L,
    };

    (void)nanosleep(&sleep_time, 0);
}