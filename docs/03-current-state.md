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

The shared device core runtime has been split so that `tk_core.c` owns core setup, task creation, scheduler startup, logging, and stop handling, while telemetry runtime behavior lives in `tk_telemetry.c`.

The STM32 target has progressed from standalone blink/serial bring-up to a minimal shared-core runtime integration on the NUCLEO-F446RE. 

The STM32 target now initializes basic board clock, LD2, and USART2, provides a minimal `tk_platform_t`, and calls `tk_core_run(&platform)`. 

The shared core starts FreeRTOS on the STM32 Cortex-M4F port and creates the current core-owned runtime tasks:
 - telemetry task 
 - status task 

The STM32 telemetry callback is currently a bring-up stub that returns a fixed telemetry sample. 

The STM32 publish callback is currently a bring-up stub that logs publish activity over USART2. 

The core-owned status task toggles the STM32 status LED through a target-provided callback. 

This is a runtime heartbeat and not product telemetry. The STM32 target does not yet read real field inputs, publish over MQTT, use LTE, load device identity, or handle certificate material.

## Device runtime idle behavior

FreeRTOS idle-hook support is enabled.

Idle behavior is target-owned. Each FreeRTOS target must provide its own `vApplicationIdleHook()` implementation.

The Python simulator target provides a POSIX idle hook that sleeps briefly while idle to avoid busy-spinning a CPU core.

## Testing state

The project now has an initial lightweight test framework across the current MVP stack.

Django uses Django's built-in test runner. The current backend test baseline covers the latest-state API behavior.

The Python simulator target uses a small pytest baseline for dependency-free simulator-side code. The initial simulator test avoids live MQTT, certificates, Docker services, and the compiled C core library.

The C device core has a CTest baseline. Pure dependency-free C logic is built in a separate `tilekoumanto_logic` target so it can be tested without linking FreeRTOS.

The current tests are intentionally small framework/bootstrap tests. Broader behavior-specific coverage is deferred until the relevant behavior is stable enough to protect.

## Developer tooling

A top-level `Makefile` provides common local development commands for stack management, per-service logs, Django tests, Python simulator build/run/tests, CTest, aggregate tests, and cleanup.

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
* STM32 real field input reading and debouncing 
* STM32 modem/LTE publishing
* STM32 identity and certificate storage 
* STM32 logging cleanup
