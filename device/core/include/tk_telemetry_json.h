#ifndef TK_TELEMETRY_JSON_H
#define TK_TELEMETRY_JSON_H

#include <stddef.h>

#include "tk_types.h"

/*
 * Returns the number of JSON characters written, excluding the null
 * terminator. Returns -1 for invalid arguments, formatting failure,
 * or insufficient output capacity.
 */

int tk_telemetry_json_serialize(
    const tk_telemetry_t *telemetry,
    char *output,
    size_t output_capacity
);

#endif