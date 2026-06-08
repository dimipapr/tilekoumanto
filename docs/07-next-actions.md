# Next Actions

## Current focus

Clarify and protect the basic device telemetry publishing model.

## Immediate next actions

1. Replace the standalone STM32 bring-up loop with a minimal STM32 target platform stub.

   Success condition:

   - STM32 target initializes board basics
   - target provides a minimal `tk_platform_t`
   - target calls `tk_core_run(&platform)`
   - LED or USART output confirms the shared core runtime is alive on the NUCLEO-F446RE

2. Keep LTE, MQTT, real field inputs, certificate storage, and modem publishing out of scope until the shared-core STM32 runtime stub works.