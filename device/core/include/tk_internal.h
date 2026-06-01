//device/core/include/tk_internal.h

#ifndef TK_INTERNAL_H
#define TK_INTERNAL_H

#include "tk_platform.h"

void tk_log(const tk_platform_t *platform, const char *format, ...);
int tk_core_stop_requested(void);

#endif