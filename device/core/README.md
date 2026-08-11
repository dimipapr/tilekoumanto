# Device Core

## Purpose

The core contains target-independent device behavior and the shared FreeRTOS application runtime.

It is used by hardware targets and the simulator through the platform contract.

## Ownership

The core owns:

- application task creation and scheduling
- telemetry sampling and publish decisions
- telemetry metadata
- canonical telemetry serialization
- monotonic runtime
- logging queue and logger task
- runtime lifecycle coordination

Targets own:

- hardware and host initialization
- field-input access
- UART, Wi-Fi, MQTT, and other transports
- wall-clock acquisition
- the final logging sink
- target-specific hooks and peripherals

## Boundary rules

- Core must not depend on target-specific APIs.
- Pure logic must not depend on FreeRTOS.
- Targets may depend on public core interfaces.
- Private runtime interfaces must not be exposed publicly.
- Core decides what and when.
- Targets implement how.
- Protocol code creates bytes.
- Transport code sends bytes.

## Telemetry flow

```text
target reads inputs
→ core constructs telemetry
→ core applies publish policy
→ core serializes telemetry
→ target transports the payload
````

Unavailable wall-clock time must not prevent telemetry publication.

## Platform contract

Platform callbacks:

* run from task context unless explicitly documented otherwise
* return `0` on success and nonzero on failure
* must not retain core-owned pointers
* must complete within a bounded amount of time
* must be thread-safe when callable from multiple tasks

The platform object must remain valid for the lifetime of the core runtime.

## Logging

* Synchronous logging is for contexts where the scheduler or logger task is unavailable.
* Queued logging is for normal runtime task use.
* Runtime producers must not wait for the physical logging transport.
* The logger task is the only runtime caller of the target logging sink.
* Interrupt logging is not supported.
* The target logging sink must not call the logging API recursively.

## Runtime constraints

* Core runtime objects use static allocation.
* Core execution is expected to be single-instance.
* Normal embedded operation does not expect the core runtime to return.
* Shutdown requests are target-provided, but shutdown coordination is core-owned.
