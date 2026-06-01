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

## Questions still open

Keep these as open:

```markdown
## Open questions

### Fault persistence

Faults are currently validated and retained in `DeviceMessageRaw.payload`.

Open question: should faults also be projected into a dedicated backend model?

### C telemetry fault model

The backend contract supports a fault list, but the current C `tk_telemetry_t` is still minimal.

Open question: how should fault details enter the C telemetry model?

### Publish timeout time source

Current publish timeout uses telemetry timestamps.

Open question: should publish timeout eventually use FreeRTOS tick time instead of wall-clock Unix time?

### Simulator behavior model

The Python simulator currently uses random input generation.

Open question: how should deterministic scenarios be represented?

### STM32 identity storage

Open question: how should the embedded target store and load provisioned identity/cert material?

### STM32 logging

Open question: what should the STM32 logging sink be during bring-up and later production firmware?

### Backend fault API

Open question: if faults get a dedicated model, how should latest-state and historical APIs expose them?
````

I would not include all low-level “how do we build STM32” questions here unless they are actually still unresolved. Keep `08` focused on decisions that affect architecture/product direction.
