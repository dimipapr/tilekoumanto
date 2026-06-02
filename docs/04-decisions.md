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


## Device runtime ownership

Decision: the shared C core owns the application runtime.

Targets initialize their platform, provide callbacks through `tk_platform_t`, and hand control to the core with:

```c
tk_core_run(&platform);
```

The core owns FreeRTOS task creation, scheduler start, telemetry loop timing, and publish decision logic.

## Target model

Decision: device targets are pluggable.

Current targets:

```text
device/targets/python-sim
device/targets/stm32
```

The Python simulator is a real target. It loads the same C core through FFI and should not duplicate core device logic.

## FreeRTOS placement

Decision: FreeRTOS lives with the shared device core.

Target builds provide the required port/config/build support, but the application runtime model belongs to core.

## Platform boundary

Decision: targets provide explicit callbacks through `tk_platform_t`.

Current callback categories:

* log
* wall-clock time
* telemetry read
* telemetry publish
* stop request

## Telemetry publish behavior

Decision: core publishes telemetry when:

* there is no previous successful publish
* `mains_power` changes
* `pump_relay` changes
* publish timeout elapses

## Device identity

Decision: device identity is certificate-backed.

The MQTT topic carries the device UUID:

```text
devices/<device_uuid>/pump/telemetry
```

The device UUID is not currently included in MQTT message metadata.

## Python simulator certificate layout

Decision: simulator certificate paths are derived from a stable cert root.

Expected layout:

```text
certs/devices/<uuid>/
├── <uuid>.crt
├── <uuid>.key
└── ca.crt
```

The simulator `.env` provides:

```env
TK_DEVICE_UUID=<uuid>
TK_MQTT_HOST=<host>
TK_MQTT_PORT=8883
TK_CERTS_ROOT=/path/to/certs
```

## MQTT telemetry contract direction

Decision: pump telemetry uses readable state values and a fault list.

The authoritative validation contract lives in:

```text
backend/django/devices/contracts/mqtt.py
```

## Device runtime timing

Telemetry message timestamps and runtime elapsed time are separate.

`tk_telemetry_t.unix_time_ms` is the device/message timestamp used in published telemetry.

Telemetry publish timeout decisions do not use `tk_telemetry_t.unix_time_ms`.

The telemetry task computes elapsed time since the last successful publish using the runtime monotonic timer and passes that elapsed duration, in milliseconds, into the publish-decision function.

The publish-decision policy remains independent of FreeRTOS-specific types.


## C logic test target

Decision: dependency-free C logic lives in a generic `tilekoumanto_logic` CMake target.

The target is intended for pure C code that does not depend on FreeRTOS, STM32 APIs, Python, MQTT, or platform callbacks.

The runtime-oriented `tilekoumanto_core` target links against `tilekoumanto_logic` and the FreeRTOS kernel.

This keeps pure logic easy to unit test while avoiding many small feature-specific logic libraries too early.