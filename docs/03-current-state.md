# Current State

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

The backend device-to-API path is working end-to-end.

Current data flow:

```text
device telemetry
→ MQTT
→ Mosquitto
→ Django MQTT catcher
→ PostgreSQL
→ API latest state response
````

The backend stack runs with Docker Compose and includes:

* Django
* PostgreSQL
* Mosquitto
* Caddy

The Django backend contains a `devices` app responsible for device state handling.

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

The project has passed basic backend path validation.

The next focus is to make the MVP contracts explicit and stable so the device side, backend side, and API behavior remain aligned.

## Known gaps

The following still need to be documented, decided, implemented, or validated before the MVP is product-ready:

* MQTT telemetry contract
* latest-state API contract
* timestamp semantics
* stale or unknown state behavior
* malformed telemetry handling
* field-device message format
* physical relay input behavior
* LTE communication behavior in realistic conditions
* field-like test procedure

## Next action

Document the MQTT telemetry contract and the latest-state API contract.
