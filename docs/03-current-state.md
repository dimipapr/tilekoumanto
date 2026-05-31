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

## Documentation state

The documentation has recently been reset and narrowed around the monitoring-only MVP.

The implementation is ahead of the refreshed documentation. Some system pieces already exist in the repository, but the refreshed docs do not yet fully describe them.

The current project work is a documentation reconciliation pass.

The goals of this pass are to:

- document what has already been implemented
- keep the MVP scope narrow
- avoid carrying over stale assumptions from older docs
- identify which implemented parts are stable
- identify which implemented parts still need review
- separate current implementation facts from future plans
- update the docs so future work can continue without reconstructing context

## Implementation state

The backend device-to-API path has been partially implemented and validated.

The MQTT ingestion path has been validated with the lightweight MQTT contract shape.

Incoming telemetry is stored in two forms:
- raw incoming messages in `DeviceMessageRaw`
- sanitized typed pump state samples in `PumpStateSample`

Device records now include a display name for easier inspection. Device UUID remains the stable identity.

The implementation also includes operator tooling under `operator/`.

`operator/project.py` exists to generate project/operator artifacts, including local certificate material for MQTT/mTLS development and testing.

Generated certificate material is local environment state and is ignored by git.

The exact current behavior of the operator tooling still needs to be documented from the implementation files before it is treated as stable project documentation.

## Current telemetry path

The MQTT catcher listens for pump telemetry messages.

Current topic pattern:

```text
devices/+/pump/telemetry
```

## Current expected message body shape:

meta.unix_time_ms
payload.mains_power_present
payload.pump_relay_active

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

## Current project focus

The current focus is documentation alignment before further implementation work.

The immediate goal is to make the refreshed docs accurately describe:

* the implemented backend state
* the implemented MQTT ingestion path
* the implemented data model
* the implemented operator/certificate tooling
* the current local deployment shape
* the known gaps before field use

After the documentation reconciliation pass, implementation can continue from a clearer baseline.

Known follow-up implementation areas include:

* MQTT catcher cleanup
* Pydantic MQTT contract review
* ingress tests
* stale-state behavior
* latest-state API alignment with the OpenAPI contract
* raw-message retention behavior

Known gaps

The following are not yet fully documented or decided in the refreshed docs:

* exact behavior and commands of operator/project.py
* generated certificate lifecycle and local/production distinction
* mapping between device identity, certificate identity, and MQTT topic authorization
* stale-state behavior for latest device state
* raw-message retention behavior
* latest-state API alignment with docs/contracts/openapi.yaml
* ingress test coverage
* whether the current API response clearly distinguishes fresh, stale, and unknown state

Documentation boundaries

This document should not contain the ordered task list for future work.

Use:

```text
03-current-state.md for what is currently true
06-implementation-log.md for chronological work history
07-next-actions.md for the next ordered actions
08-open-questions.md for unresolved decisions and uncertainties
```