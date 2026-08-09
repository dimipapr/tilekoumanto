#ifndef TK_COMMS_H
#define TK_COMMS_H

int tk_comms_init(void);
int tk_comms_is_connected(void);
int tk_comms_mqtt_is_connected(void);

int tk_comms_publish(
    const char *topic,
    const char *payload
);

#endif