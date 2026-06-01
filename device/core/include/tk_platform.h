#ifndef TK_PLATFORM_H
#define TK_PLATFORM_H

#include <stdint.h>

typedef struct {
    void (*log)(const char *message);
    uint64_t (*unix_time_ms)(void);
} tk_platform_t;

#endif