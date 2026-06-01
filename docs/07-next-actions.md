# Next Actions

## Current focus

Stabilize the shared device core and Python simulator path before returning to STM32 integration.

The current active path is:

```text
python-sim target
→ shared C core through FFI
→ MQTT over mTLS
→ Django MQTT catcher
→ backend storage
```

## Immediate next actions

1. Separate FreeRTOS-free telemetry policy from the telemetry runtime task.

   Current state:

   * `tk_core.c` owns core setup, task creation, scheduler startup, logging, and stop handling.
   * Telemetry behavior has been moved out of `tk_core.c`.
   * Telemetry publish timeout now uses elapsed runtime time passed as `uint64_t` milliseconds.
   * `tk_should_publish_telemetry()` is already close to FreeRTOS-free core policy.

   Next cleanup:

   * Keep FreeRTOS-free telemetry decision logic separate from the FreeRTOS task/runtime adapter.
   * Keep `tk_should_publish_telemetry()` independent of FreeRTOS types.
   * Move or isolate FreeRTOS-specific task logic, tick conversion, and platform callback usage from pure telemetry policy.
   * Preserve current behavior while improving testability and module boundaries.

   Possible file direction:

   ```text
   device/core/src/tk_telemetry.c          # FreeRTOS-free telemetry policy
   device/core/src/tk_telemetry_task.c     # FreeRTOS task/runtime adapter
   device/core/include/tk_telemetry.h


2. Add deterministic Python simulator scenarios.

   Random input generation is useful for smoke testing, but repeatable scenarios are needed for validation.

   Initial scenarios:

   * steady state
   * mains power loss
   * mains power restore
   * pump relay active
   * pump relay inactive
   * unreadable mains input
   * unreadable pump relay input

3. Add simulator fault generation.

   The backend contract supports a fault list, but the simulator currently publishes no faults.

4. Decide how faults should be represented beyond raw-message retention.

   Current state:

   * faults are validated
   * faults are retained in `DeviceMessageRaw.payload`
   * faults are not projected into a dedicated backend model

5. Return to STM32 integration.

   The STM32 target should implement `tk_platform_t` and hand control to the shared core through `tk_core_run()`.

## Known follow-up areas

* MQTT catcher cleanup
* ingress tests
* stale-state behavior
* latest-state API alignment with the OpenAPI contract
* raw-message retention policy
* deterministic simulator scenarios
* simulator fault generation
* STM32 shared-core integration
* real input debouncing
* real modem/network publishing