# Next Actions

## 1. Refactor MQTT catcher

Keep the MQTT callback small by separating the ingress flow into focused units:

- topic/device UUID extraction
- JSON decoding
- raw message storage
- Pydantic validation
- typed pump state sample creation
- device `last_seen` update

Goal: the MQTT callback should describe the flow, not contain all implementation details.

## 2. Tighten Pydantic MQTT contract models

Review the existing Pydantic models and make sure they clearly represent:

- shared MQTT message metadata
- pump telemetry payload
- full pump telemetry message body

The Pydantic models are the executable MQTT message body contract.

## 3. Add ingress tests

Add tests for the MQTT ingestion path:

- valid pump telemetry message
- invalid JSON
- unknown device UUID
- missing `meta`
- missing or invalid `meta.unix_time_ms`
- missing `payload`
- wrong `payload.mains_power_present` type
- wrong `payload.pump_relay_active` type
- raw message saved but typed sample rejected where appropriate

## 4. Decide raw-message retention behavior

Decide whether raw MQTT messages should be kept forever during MVP development or whether a cleanup command should exist for test/dev data.

## 5. Decide stale-state behavior

Define when a device state becomes stale based on backend receive time.

## 6. Align latest-state API

Update the latest-state API to read from `PumpStateSample` and match the API contract.