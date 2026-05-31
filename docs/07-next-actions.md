# Next Actions

## Current mode

Resume monitoring MVP from the device side.

## 1. Define the first device telemetry sender

Create the smallest device-side implementation that can publish the current MVP telemetry message to MQTT.

The first device-side goal is not full firmware.

The goal is to produce one valid pump telemetry message using the current backend MQTT contract:

```text
devices/{device_uuid}/pump/telemetry
```