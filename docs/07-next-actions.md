## Current focus

Replace the STM32 bring-up telemetry stub with real digital input readings.

## Immediate next actions

1. Select NUCLEO-F446RE GPIO pins for the MVP `mains_power` and `pump_relay` inputs.
2. Configure and read those pins in the STM32 target.
3. Map the raw input states into the existing `tk_telemetry_t` model.
4. Validate that physical input changes exercise the existing shared-core publish-decision behavior.

Keep ESP32 transport, MQTT, debouncing, and production input circuitry out of this step.