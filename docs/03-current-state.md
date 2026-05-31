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

## Documentation state

The documentation has recently been reset and narrowed around the monitoring-only MVP.

The implementation is ahead of the refreshed documentation. The current project work is a documentation reconciliation pass: document what already exists, keep the MVP narrow, and separate current implementation facts from future plans and unresolved questions.

## Implementation state

The backend device-to-API path has been partially implemented and validated.

The MQTT ingestion path has been validated with the lightweight MQTT contract shape.

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

The MQTT catcher listens for pump telemetry messages on this topic pattern:

```text
devices/+/pump/telemetry
```

The current expected message body shape is:

```text
meta.unix_time_ms
payload.mains_power_present
payload.pump_relay_active
```

The MQTT topic device segment is used to look up the backend `Device.uuid`.

## Current interface

For the MVP, the API is the only farmer-facing interface.

No separate frontend or mobile application exists yet.

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

## Known follow-up implementation areas

Known follow-up implementation areas include:

* MQTT catcher cleanup
* Pydantic MQTT contract review
* ingress tests
* stale-state behavior
* latest-state API alignment with the OpenAPI contract
* raw-message retention behavior
* ingress tests
