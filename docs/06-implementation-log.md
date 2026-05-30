## 2026-05-30

Confirmed MQTT ingestion using the lightweight MQTT contract shape.

A test message published to:

```text
devices/{device_uuid}/pump/telemetry
````

with envelope:

```text
meta.unix_time_ms
payload.mains_power_present
payload.pump_relay_active
```

was consumed by the Django MQTT catcher and stored as a raw database row.

The MQTT catcher now reads `device_uuid` from the topic and reads message time from `meta.unix_time_ms`.

## 2026-05-30

Added typed pump state ingestion.

MQTT pump telemetry messages are now stored in two forms:

- raw incoming message in `DeviceMessageRaw`
- sanitized typed state sample in `PumpStateSample`

`PumpStateSample` stores:

- device timestamp
- backend receive timestamp
- mains power state
- pump relay state

Device display names were added to make admin inspection easier.
