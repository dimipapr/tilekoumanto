# Current State

## Maintenance rule

Update this document only when the stable project state changes.

Routine work, debugging details, chronological notes, and small implementation notes belong in `06-implementation-log.md`.

Unresolved decisions and uncertainties belong in `08-open-questions.md`.

## Product scope

Tilekoumanto is currently focused on a monitoring-only MVP for one field and one pump.

The MVP is intended to expose:

* mains power state
* pump relay state
* latest known device state through the API

The MVP does not currently include:

* remote start/stop control
* pressure monitoring
* notifications
* automation or scheduling
* multi-field management
* separate web dashboard
* mobile application

## Implementation state

The backend device-to-API path has been partially implemented and validated.

The current device telemetry path has also been validated through the Python simulator target, which runs the shared C device core and publishes telemetry over MQTT/mTLS.

The MQTT ingestion path has been validated with the current pump telemetry contract.

Incoming telemetry is stored in two forms:

- raw incoming messages in `DeviceMessageRaw`
- sanitized typed pump state samples in `PumpStateSample`

Raw messages store the MQTT topic, payload, received time, and optional device-reported Unix timestamp.

Pump state samples store validated mains power and pump relay state linked back to the raw message.

Device records include a `display_name` for easier inspection. Device `uuid` remains the stable device identity.

Telemetry is consumed by the Django backend and stored for use by a latest-state API.

The latest-state API is implemented at `GET /api/devices/{device_uuid}/state`.

## Admin inspection

The Django admin registers `Device`, `DeviceMessageRaw`, and `PumpStateSample` for operator/developer inspection.

The admin device list includes a derived connection status based on `last_seen`.

This admin status is currently inspection-only and is not the documented latest-state API stale-state contract.

## Local deployment

The local deployment uses Docker Compose services for:

* Caddy
* Mosquitto
* PostgreSQL
* Django web API
* Django MQTT catcher

Caddy exposes HTTPS for `dev.tilekoumanto.gr`, serves static files, provides `/gateway/health`, and proxies application traffic to the Django web service.

Mosquitto has two listeners:

* internal cleartext MQTT on port `1883` for backend services
* external MQTT over TLS on port `8883` for edge devices

The external MQTT listener requires client certificates and uses the certificate identity as the MQTT username.

The Django web service and MQTT catcher both connect to PostgreSQL using environment variables.

The generated device manifest is currently used to provision Django `Device` records.

## Operator tooling

The repository includes operator tooling for generating local MQTT/mTLS certificate material.

The command is:

```bash
python operator/project.py certs <target_count>
```

Generated certificate material is written to the top-level `certs/` directory, which is ignored by git and treated as local environment state.

The tooling generates:

* a local CA
* Mosquitto server certificate material
* device client certificate material
* a device manifest

Generated device identities are UUIDs. Each generated device certificate uses its UUID as the certificate common name.

## Current telemetry path

The current telemetry path is working end-to-end through the Python simulator.

```text
python-sim target
→ shared C device core through FFI
→ MQTT over mTLS
→ Django MQTT catcher
→ backend storage
```
The MQTT topic carries the device UUID.

The backend validates incoming telemetry, stores the full raw message, and projects the current pump state into the existing sample table.

Fault data is currently retained in the raw message payload. Dedicated fault persistence is not implemented yet.

## Current interface

A simple `/devices` operator page exists for development inspection.

It lists known devices, shows each device latest pump state when available, links to the latest-state JSON endpoint, and orders devices by latest received sample.

This page is operator/developer tooling and is not a separate farmer-facing dashboard.

## Device implementation state

The shared device core now exists under `device/core`.

The core owns the application runtime and uses FreeRTOS. Targets provide platform callbacks and hand control to the core through `tk_core_run()`.

The Python simulator target under `device/targets/python-sim` is the current active validation target. It loads the C core through FFI, simulates device inputs, and publishes real MQTT telemetry over mTLS.

The STM32 target scaffold exists under `device/targets/stm32`, but runtime integration with the shared core is still pending.

## Known follow-up implementation areas

* deterministic Python simulator scenarios
* simulator fault generation
* dedicated backend fault persistence decision
* MQTT catcher cleanup
* ingress tests
* stale-state behavior
* latest-state API alignment with the OpenAPI contract
* raw-message retention behavior
* STM32 integration with the shared device core
