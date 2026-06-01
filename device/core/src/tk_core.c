#include "tk_core.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#define TK_LOG_BUFFER_SIZE 128

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

int tk_core_version(void)
{
    return 1;
}

int tk_core_probe_platform(const tk_platform_t *platform)
{
    uint64_t now;

    if (platform == 0) {
        return 0;
    }

    if (platform->log == 0) {
        return 0;
    }

    if (platform->unix_time_ms == 0) {
        return 0;
    }

    tk_log(platform, "core platform probe starting");

    now = platform->unix_time_ms();

    if (now == 0U) {
        tk_log(platform, "core platform time callback returned zero");
        return 0;
    }

    tk_log(platform, "core platform time ms: %" PRIu64, now);
    tk_log(platform, "core platform probe complete");

    return 1;
}