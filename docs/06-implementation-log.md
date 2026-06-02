## 2026-05-30

Confirmed MQTT ingestion using the lightweight MQTT contract shape.

A test message published to:

```text
devices/{device_uuid}/pump/telemetry
````

with envelope:

```text
meta.unix_time_ms
payload.mains_power_present
payload.pump_relay_active
```

was consumed by the Django MQTT catcher and stored as a raw database row.

The MQTT catcher now reads `device_uuid` from the topic and reads message time from `meta.unix_time_ms`.

## 2026-05-30

Added typed pump state ingestion.

MQTT pump telemetry messages are now stored in two forms:

- raw incoming message in `DeviceMessageRaw`
- sanitized typed state sample in `PumpStateSample`

`PumpStateSample` stores:

- device timestamp
- backend receive timestamp
- mains power state
- pump relay state

Device display names were added to make admin inspection easier.

## 2026-05-30

Improved MQTT ingress storage, validation, and admin inspection.

The backend now stores incoming pump telemetry in two forms:

- raw incoming MQTT messages in `DeviceMessageRaw`
- sanitized typed pump state samples in `PumpStateSample`

`DeviceMessageRaw` is used as the raw/debug record. It stores the original decoded JSON payload, MQTT topic, backend receive time, and the device-provided Unix timestamp value when available.

`PumpStateSample` is used as the typed product record. It stores the processed pump state fields used by application logic:

- device timestamp
- backend receive timestamp
- mains power state
- pump relay state

The MQTT catcher now reads the device UUID from the MQTT topic:

```text
devices/{device_uuid}/pump/telemetry
````

The current message body shape is:

```text
meta.unix_time_ms
payload.mains_power_present
payload.pump_relay_active
```

Pydantic models are being used as the executable MQTT message body contract.

Django admin improvements:

* device records now have display names
* device list sorts by latest `last_seen` first, with null values last
* raw messages are paginated for safer inspection
* pump state samples show a link to the related raw message
* ingestion records are treated as read-only inspection records

Confirmed behavior:

```text
MQTT message received
→ device UUID extracted from topic
→ JSON payload decoded
→ raw message saved
→ pump telemetry validated
→ typed pump state sample saved
→ admin inspection available
```

## 2026-05-31 Operator Certificate tooling reviewed

Reviewed `operator/project.py`, `operator/lib/certs.py`, and `.gitignore`.

Confirmed that the operator CLI can generate local MQTT/mTLS certificate material under the top-level `certs/` directory, which is ignored by git.

The tooling generates a local CA, Mosquitto server certificate material, device client certificate material, and a device manifest. Device certificate identities are generated as UUIDs.

Remaining questions about certificate-to-device mapping, Mosquitto authentication, topic authorization, and production use were left for local deployment/backend review.

## 2026-05-31 Local deployment reviewed

Reviewed Docker Compose, Mosquitto config, Caddy config, and Django settings.

Confirmed that the local deployment includes Caddy, Mosquitto, PostgreSQL, Django web, and a separate Django MQTT catcher service.

Confirmed that Mosquitto has an internal anonymous cleartext listener on port `1883` and an external mTLS listener on port `8883`.

Noted that external MQTT uses client certificates and `use_identity_as_username`.

Identified remaining questions around per-device topic authorization, Django device loading from the generated manifest, and the distinction between current local `runserver` behavior and intended production serving.

## 2026-05-31 API contract reviewed

Reviewed `docs/contracts/openapi.yaml`, `config/urls.py`, `devices/urls.py`, and `devices/views.py`.

Confirmed that the documented latest-state endpoint `GET /api/devices/{device_uuid}/state` is not implemented yet.

The currently exposed Django device API only includes `GET /api/health/`.

Implementation should not continue until the docs clearly mark latest-state API alignment as pending.

## 2026-05-31 Django admin and runtime files reviewed

Reviewed Django admin, tests placeholder, Django Dockerfile, and Caddy Dockerfile.

Confirmed that Django admin exposes `Device`, `DeviceMessageRaw`, and `PumpStateSample` for inspection.

Confirmed that the device admin derives an inspection-only connection status from `last_seen` using a five-minute threshold.

Confirmed that `devices/tests.py` currently has no implemented tests.

## 2026-05-32 Latest-state API implementation

Implemented `GET /api/devices/{device_uuid}/state`.

The endpoint returns the latest `PumpStateSample` for a known `Device.uuid`, selected by backend `received_at`.

Added focused tests for:

- latest state response
- unknown device UUID
- known device with no pump state samples

Added a Pydantic API response contract for latest-state serialization.

Adjusted the raw-message relationship so typed pump state samples can remain usable independently of raw debug messages.

## 2026-05-31 ingest telemetry sim and devices list page

Added a temporary Python device telemetry simulator.

The simulator:

- reads generated device identities from `certs/devices/manifest.json`
- selects one generated device UUID
- uses that device certificate material from `certs/devices/{device_uuid}/`
- publishes pump telemetry to `devices/{device_uuid}/pump/telemetry`
- sends randomized `mains_power_present` and `pump_relay_active` values
- publishes every 10 seconds
- is development-only and not production firmware

Confirmed that simulator messages update the latest-state API.

Added a simple `/devices` operator page.

The page:

- lists known devices
- shows display name and UUID
- shows latest received pump state
- shows latest sample timestamps
- links to each device latest-state JSON endpoint
- orders devices by latest received sample, newest first

This page is development/operator tooling, not a separate farmer-facing dashboard.


## 2026-06-01 Device core runtime and Python simulator validation

Implemented the first shared device-core runtime path.

The C core now owns the FreeRTOS runtime model and exposes `tk_core_run()` as the target handoff entrypoint.

The Python simulator target now:

- builds the shared C core as a target-local shared library
- loads the core through `ctypes`
- provides platform callbacks for log, time, telemetry read, telemetry publish, and stop request
- simulates telemetry input states
- publishes real MQTT telemetry over mTLS
- supports clean shutdown from the simulator process

The core currently publishes telemetry when:

- there is no previous successful publish
- `mains_power` changes
- `pump_relay` changes
- the publish timeout elapses

Aligned the backend MQTT contract with the device-side readable state model.

The current pump telemetry message shape is:

```json
{
  "meta": {
    "unix_time_ms": 1780318912573
  },
  "payload": {
    "readings": {
      "mains_power": "present",
      "pump_relay": "inactive"
    },
    "faults": []
  }
}
```

Current reading values:

```text
mains_power: present | not_present | fault
pump_relay: active | inactive | fault
```

Current fault values:

```text
target: mains_power | pump_relay
type: unreadable
```

The MQTT catcher now validates the updated contract, saves the full raw payload, and projects the current readings into the existing `PumpStateSample` boolean fields.

Faults are currently validated and retained in the raw message payload, but they are not persisted in a dedicated fault model yet.

## 2026-06-01 Device core cleanup and telemetry timing separation

Split the shared device core runtime so `tk_core.c` focuses on platform setup, task creation, scheduler startup, logging, and stop handling.

Moved telemetry task behavior and telemetry publish-decision logic into telemetry-specific core files.

The current core-created application task is the telemetry task.

Changed telemetry publish timeout handling so timeout decisions use elapsed runtime time since the last successful publish instead of subtracting telemetry message timestamps.

`tk_telemetry_t.unix_time_ms` remains the telemetry message/event timestamp.

The publish-decision function now receives elapsed milliseconds as an explicit `uint64_t`, keeping the policy independent of FreeRTOS-specific types.

Split FreeRTOS into a separate CMake target and tightened compile warnings for project core code.

## 2026-06-02 Testing framework scafolding

Started a lightweight test framework across the current MVP stack.

Django testing was kept on Django's built-in test runner. The existing latest-state API tests were cleaned up and kept as the initial backend test baseline.

Added a small Python simulator pytest baseline with a dependency-free test. The simulator test avoids live MQTT, certificates, Docker services, random simulator behavior, and the compiled C core library.

Split the C device core so pure telemetry decision logic is separated from the FreeRTOS telemetry task/runtime adapter.

Added a generic tilekoumanto_logic CMake target for dependency-free C logic.

tilekoumanto_core now links against:`tilekoumanto_logic, freertos_kernel`

Added an initial CTest-based C test executable to verify compiling linking and calling pure C logic