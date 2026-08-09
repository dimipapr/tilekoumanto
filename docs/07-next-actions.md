## Current focus

Code cleanup and organization.

## Next actions

1. Move STM32 UART reception and forwarding out of the ESP32 `app/main.c`.
2. Separate STM32 logging, coprocessor communication, and telemetry serialization from the STM32 `app/main.c`.
3. Preserve the validated runtime behavior while moving code.
4. Add focused host tests for telemetry serialization after it is extracted.
5. Rebuild, flash, and validate the end-to-end path after each cleanup step.

Do not add timestamps, acknowledgements, retries, offline buffering, Ethernet, or cellular support during cleanup.
