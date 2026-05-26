#include "tk_hal.h"
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>

// Internal State and callbacks

static char device_uuid[64] = {0};
static char topic[128] = {0};
static char mqtt_payload[256] = {0};

static hal_get_telemetry_cb get_telemetry = NULL;
static hal_mqtt_publish_cb mqtt_publish = NULL;
static hal_get_unix_time_cb get_unix_time = NULL;
static hal_lock_cb lock = NULL;
static hal_unlock_cb unlock = NULL;

void tk_core_init(
    const char *_uuid,
    hal_get_telemetry_cb tel_cb,
    hal_mqtt_publish_cb pub_cb,
    hal_get_unix_time_cb time_cb,
    hal_lock_cb lock_cb,
    hal_unlock_cb unlock_cb
){
    if (_uuid) {
        strncpy(device_uuid, _uuid, sizeof(device_uuid) - 1);
        device_uuid[sizeof(device_uuid) - 1] = '\0';
    }
    get_telemetry = tel_cb;
    mqtt_publish = pub_cb;
    get_unix_time = time_cb;
    lock = lock_cb;
    unlock = unlock_cb;
    snprintf(topic, sizeof(topic), "devices/%s/pump/telemetry", device_uuid);
}

#define POLL_INTERVALS_MS       100
#define TELEMETRY_INTERVALS_MS  60000

static uint64_t last_poll_time = 0;
static uint64_t last_publish_time = 0;
static tk_telemetry_t last_published_state = {0};

void tk_core_tick(void){
    //ensure required callbacks exist
    if (!get_unix_time || !get_telemetry)return;

    uint64_t now = get_unix_time();

    // Fast loop
    if (now - last_poll_time >= POLL_INTERVALS_MS){
        last_poll_time = now;

        tk_telemetry_t current_state = {0};

        if (lock) lock();
        get_telemetry(&current_state);
        if (unlock) unlock();
        bool state_changed = (current_state.relay_active != last_published_state.relay_active) || 
                             (current_state.mains_present != last_published_state.mains_present);
        if ( state_changed || now - last_publish_time >= TELEMETRY_INTERVALS_MS ){
            snprintf(
                mqtt_payload,
                sizeof(mqtt_payload),
                "{\"metadata\":{\"device_uuid\":\"%s\",\"timestamp\":%"PRIu64"},"
                "\"payload\":{\"mains_present\":%s,\"relay_active\":%s}}",
                device_uuid, 
                now, 
                current_state.mains_present ? "true" : "false", 
                current_state.relay_active ? "true" : "false"
            );
            mqtt_publish(topic, mqtt_payload);
            last_published_state.mains_present = current_state.mains_present;
            last_published_state.relay_active = current_state.relay_active;
            last_publish_time = now;
        }
    }
    return;
}

