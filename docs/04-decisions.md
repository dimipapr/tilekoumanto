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

## Latest-state API timestamp behavior

The latest-state API returns backend and device timestamps but does not compute stale-state status.

For the current MVP implementation, the backend determines the latest pump state sample by `PumpStateSample.received_at`.

`PumpStateSample.device_timestamp` represents the device-reported event time and is retained for future clock validation, delay estimation, and possible event-time ordering.

For the MVP, clients decide whether state is stale using the returned timestamps.

## API response contracts use Pydantic

API response shapes should be represented with small Pydantic models where useful.

For the MVP, Pydantic is used to keep response serialization and documented API shape aligned.

The lightweight `docs/contracts/openapi.yaml` is removed for now.

## C standard
C99