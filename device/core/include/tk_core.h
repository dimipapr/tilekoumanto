// device/core/include/tk_core.h

#ifndef TK_CORE_H
#define TK_CORE_H

#include <stdint.h>

#include "tk_platform.h"

int tk_core_version(void);
int tk_core_run(const tk_platform_t *platform);

#endif