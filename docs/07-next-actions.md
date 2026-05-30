# Next Actions

## 1. Clean up MQTT catcher structure

Move parsing, validation, and database writes into small functions so the MQTT callback stays simple.

## 2. Add ingress tests

Add tests for:

- valid pump telemetry message
- invalid JSON
- missing `meta`
- missing `meta.unix_time_ms`
- missing `payload`
- wrong payload field types
- unknown device UUID

## 3. Decide stale-state behavior

Define when a device state becomes stale based on backend receive time.

## 4. Align latest-state API

Update the latest-state API to read from `PumpStateSample` and match `docs/contracts/api.yaml`.

## 5. Prepare field-like ingress test

Test publishing through the external MQTT mTLS path and confirm the message reaches the same raw and typed database tables.