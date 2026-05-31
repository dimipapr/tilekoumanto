# Next Actions

## Current mode

Start adding nice shit

## 1. Harden MQTT payload ingestion

Update `mqtt_catcher.py` so malformed MQTT payloads are handled intentionally.

Current assumptions:

- MQTT device authentication happens at the broker level.
- MQTT topic authorization should happen through broker ACLs.
- This worker normally receives authenticated device messages on valid telemetry topics.
- A separate catch-all or security-monitoring worker may be considered later, but is not part of this task.

For this task, keep the existing behavior shape:

- invalid topic: log and discard
- unknown device: log and discard
- malformed payload: log and discard
- invalid pump telemetry contract: save raw message, log, and discard typed sample
- valid pump telemetry contract: save raw message and typed `PumpStateSample`

Add payload sanitization before JSON parsing and typed validation:

- reject payloads over a small maximum byte size
- reject non-UTF-8 payloads
- reject invalid JSON
- reject JSON values that are not objects

The MQTT worker must not crash for malformed input. Bad input should be logged and discarded.