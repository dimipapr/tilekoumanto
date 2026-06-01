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
````

## Immediate next actions

1. Commit the current runtime, simulator, backend contract, and documentation updates.

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