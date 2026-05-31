## Operator certificate material

Generated certificate material is written to the top-level `certs/` directory.

The `certs/` directory is ignored by git and treated as local environment state.

Generated device certificate identities are UUIDs.

## MQTT listeners

The Mosquitto deployment uses two listeners:

- port `1883` for internal Docker-network backend traffic
- port `8883` for external edge-device MQTT over mTLS with client certificates required

## Latest-state API MVP behavior

Accepted for MVP:

- Unknown device UUID returns HTTP 404.
- Known device with no pump state samples returns HTTP 404.
- `state_is_stale` is true when the latest sample `received_at` is older than 5 minutes.
- `device_reported_unix_time_ms` is read from the raw message linked to the latest pump state sample.

## Application state should not depend on raw messages

Raw MQTT messages are retained for debugging, audit, and operator inspection only.

After ingress has validated and converted an incoming telemetry message into typed application records, normal application code should use those typed records instead of reading fields back from `DeviceMessageRaw`.

For pump status, application/API code should use `PumpStateSample`.

`DeviceMessageRaw` may still be linked from typed records so developers/operators can inspect the original incoming message when debugging.