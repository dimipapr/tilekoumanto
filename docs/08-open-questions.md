# Open Questions

## Device identity and MQTT authorization

- Does Mosquitto currently enforce per-device topic authorization?
- Should Mosquitto ACLs be added so each device certificate identity can only publish to its own topic?

## Device provisioning

- Is the current certificate tooling local-development-only, or also a basis for production provisioning?
- Should unknown-device telemetry be retained as raw messages, or is dropping it before raw storage the intended behavior?

## Local deployment

- Should local deployment continue using Django `runserver`, or should production serving be documented separately?
- Should `SECURE_PROXY_SSK_HEADER` be corrected to `SECURE_PROXY_SSL_HEADER`?

## API contract alignment

- Should `GET /api/devices/{device_uuid}/state` be implemented next using the latest `PumpStateSample` for the requested `Device.uuid`?
- How should `state_is_stale` be calculated?
- What should the API return when the device exists but has no pump state samples yet?
- What should the API return when the device UUID is unknown?

## Device timestamp as primary sample time

Consider making `PumpStateSample.device_timestamp` the primary time for ordering pump state samples after device clock validation is defined.

Current MVP latest-state selection uses backend `received_at`.

Future behavior may use `device_timestamp` once the system can validate or trust device clock behavior well enough.

Open considerations:

- how to detect bad device clocks
- how to handle delayed or out-of-order MQTT messages
- whether `received_at` remains the ingestion/audit time only
- how to expose clock skew or delay to clients