## Operator certificate tooling

- Is the device certificate CN expected to match the Django `Device.device_uuid`?
- Is the device certificate CN expected to match the MQTT topic device segment?
- Does Mosquitto currently enforce client certificate authentication?
- Does Mosquitto currently enforce per-device topic authorization?
- Is `certs/devices/manifest.json` the intended source for loading devices into Django?
- Is the current certificate tooling local-development-only, or also a basis for production provisioning?