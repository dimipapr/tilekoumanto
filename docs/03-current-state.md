# Current State

## Maintenance rule

Update this document only when the stable project state changes. Routine work, debugging details, and small implementation notes belong in `06-implementation-log.md`.

## Product scope

Tilekoumanto is currently focused on a monitoring-only MVP for one field and one pump.

The MVP exposes:

- mains power state
- pump relay state
- latest known device state through the API

The MVP does not currently include:

- remote start/stop control
- pressure monitoring
- notifications
- automation or scheduling
- multi-field management
- separate web dashboard
- mobile application

## Implementation state

The MQTT ingestion path has been validated with the lightweight MQTT contract shape 

incoming telemetry is stored in two forms:
- raw incoming messages in `DeviceMessageRaw`
- sanitized typed pump state samples in `PumpStateSample`

Device records now include a display name for easier inspection. Device UUID remains the stable identity.

## Current telemetry path

The MQTT catcher listens for pump telemetry messages.

Current topic pattern:

```text
devices/+/pump/telemetry
```

Telemetry is consumed by the Django backend and made available through the API as the latest known device state.

## Current interface

For the MVP, the API is the only farmer-facing interface.

No separate frontend or mobile application exists yet.

## Current project focus

The backend device-to-API path is working.

The current focus is to make MQTT ingress reliable, simple, and easy to inspect.

The latest-state API contract exists, but API alignment is intentionally deferred while ingress is being cleaned up.

- MQTT catcher validation and parsing should be cleaned into smaller functions
- latest-state API still needs to be aligned with `docs/contracts/api.yaml`
- stale-state behavior still needs to be defined
- ingress tests still need to be added

## Next action

Align api with api contract
