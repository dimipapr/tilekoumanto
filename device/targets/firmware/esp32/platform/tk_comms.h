#ifndef TK_COMMS_H
#define TK_COMMS_H

#include <stdbool.h>

#include "esp_err.h"

esp_err_t tk_comms_init(void);
bool tk_comms_is_ready(void);

int tk_comms_publish(
    const char *topic,
    const char *payload
);

#endif