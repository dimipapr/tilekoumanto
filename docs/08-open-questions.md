# Open Questions

## Device identity and MQTT authorization

- Is the device certificate CN expected to match the Django `Device.device_uuid`?
- Is the device certificate CN expected to match the MQTT topic device segment?
- Does Mosquitto currently enforce per-device topic authorization?
- Should Mosquitto ACLs be added so each device certificate identity can only publish to its own topic?

## Device provisioning

- Is `certs/devices/manifest.json` the intended source for loading devices into Django?
- Is the current certificate tooling local-development-only, or also a basis for production provisioning?

## Local deployment

- Should local deployment continue using Django `runserver`, or should production serving be documented separately?
- Should `SECURE_PROXY_SSK_HEADER` be corrected to `SECURE_PROXY_SSL_HEADER`?