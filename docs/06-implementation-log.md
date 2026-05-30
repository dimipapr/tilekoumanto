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

