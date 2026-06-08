# Device Subsystem Design

## Purpose

This document describes the current device-side subsystem design.

The device subsystem observes field-side pump signals, converts those observations into telemetry, and publishes that telemetry to the backend.

The current implementation focus is telemetry-only monitoring.

Remote control is intentionally out of scope for the current MVP.

## Current hardware direction

The current development target is:

```text
NUCLEO-F446RE
````

The current cellular module direction is:

```text
M5Stack U111-B
```

The NUCLEO-F446RE is used as a capable STM32 development target for exploration. Final product hardware may be reduced later once firmware and product requirements are clearer.

## Current signal model

The first field signal source is an auxiliary or dry contact from external electrical equipment.

Current monitored states:

```text
mains_power
pump_relay
```

Current reading values:

```text
mains_power: present | not_present | fault
pump_relay: active | inactive | fault
```

The current MVP does not directly measure:

* per-phase voltage
* per-phase current
* hydraulic pressure
* dry-run risk
* bearing wear
* water flow

Those are possible later extensions after the basic device-to-backend telemetry path is stable.

## Telemetry behavior

The device publishes telemetry when any of the following is true:

* there is no previous successful publish
* `mains_power` changed
* `pump_relay` changed
* the publish timeout elapsed

The device does not publish every input sample if the state is unchanged and the publish timeout has not elapsed.

Telemetry message timestamps and runtime elapsed time are separate.

`tk_telemetry_t.unix_time_ms` is the telemetry message or event timestamp.

Publish timeout decisions use elapsed runtime time since the last successful publish, not subtraction between telemetry message timestamps.

## Runtime architecture

The device-side implementation is split into a shared C core and pluggable targets.

```text
device/
├── core/
└── targets/
    ├── python-sim/
    └── stm32/
```

The shared device core owns the application runtime model.

Current targets provide platform-specific setup and callbacks, then hand control to the core.

The current core-owned application task graph contains:

```text
telemetry task
status task
```

The telemetry task owns periodic telemetry processing and publish-decision execution.

The status task owns a simple runtime heartbeat and calls a target-provided status LED callback when available.

Targets should not duplicate core device decision logic.

## C layering

The device core separates dependency-free logic from runtime/task integration.

Current CMake layering:

```text
tilekoumanto_logic
  Pure C logic.
  No FreeRTOS dependency.
  No STM32 dependency.
  No Python dependency.
  No MQTT dependency.
  Intended to be easy to unit test.

tilekoumanto_core
  Runtime-oriented core library.
  Links tilekoumanto_logic.
  Links FreeRTOS.
  Owns task/runtime integration.
```

Current source split:

```text
device/core/src/tk_telemetry.c
  Telemetry decision logic.

device/core/src/tk_telemetry_task.c
  FreeRTOS telemetry task/runtime adapter.

device/core/src/tk_status_task.c
  FreeRTOS status LED/runtime heartbeat task.
```

The purpose of this split is to keep device decision logic testable without linking the FreeRTOS runtime.

## Shared core responsibilities

The shared C core is responsible for:

* owning the FreeRTOS application runtime model
* exposing `tk_core_run()` as the target handoff entrypoint
* creating core-owned tasks
* running the telemetry task
* running the status task
* reading telemetry through the platform interface
* calling dependency-free telemetry logic to decide whether telemetry should be published
* remembering the last successfully published telemetry sample
* calling target-provided publish behavior
* calling target-provided status LED behavior when available

The core must not depend on:

* STM32 LL APIs
* GPIO registers
* UART registers
* modem AT commands
* Python runtime details
* simulator-specific behavior
* backend database details

## Target responsibilities

Targets are responsible for:

* hardware or simulator initialization
* implementing the platform callback table
* providing wall-clock Unix time or a temporary bring-up timestamp source
* reading or simulating telemetry inputs
* publishing telemetry through the target transport
* implementing optional status LED behavior
* handling target-specific shutdown behavior when applicable

A target initializes enough platform state for the core to run, then calls:

```c
tk_core_run(&platform);
```

Normal embedded operation should not expect `tk_core_run()` to return.

## Platform callback interface

The core calls target-provided behavior through `tk_platform_t`.

Current callback shape:

```c
typedef struct {
    void (*log)(const char *message);
    uint64_t (*unix_time_ms)(void);
    int (*read_telemetry)(tk_telemetry_t *out);
    int (*publish_telemetry)(const tk_telemetry_t *telemetry);
    int (*should_stop)(void);
    int (*status_led_toggle)(void);
} tk_platform_t;
```

Current callback meanings:

```text
log
  Target-provided logging sink.

unix_time_ms
  Target-provided telemetry timestamp source.
  On real embedded targets this should eventually be real wall-clock or device event time.
  During STM32 bring-up this may temporarily use core runtime milliseconds.

read_telemetry
  Target-provided telemetry input reader.
  The target fills a `tk_telemetry_t` sample.

publish_telemetry
  Target-provided publish operation.
  The core decides when to publish.
  The target decides how to publish.

should_stop
  Target-provided stop request.
  Mainly used by simulator targets for clean shutdown.
  Embedded targets can return 0 forever.

status_led_toggle
  Optional target-provided status LED toggle.
  Used by the core-owned status task when available.
```

Platform callbacks that return `int` use C-style status conventions.

For `read_telemetry`, `publish_telemetry`, and `status_led_toggle`:

```text
0 = success
nonzero = failure
```

For `should_stop`:

```text
0 = continue running
nonzero = stop requested
```

## Shared telemetry type

The current shared telemetry sample contains:

```text
mains_power
pump_relay
unix_time_ms
seq
```

Current conceptual shape:

```c
typedef struct {
    tk_mains_power_state_t mains_power;
    tk_pump_relay_state_t pump_relay;
    uint64_t unix_time_ms;
    uint32_t seq;
} tk_telemetry_t;
```

Current enum meanings:

```text
tk_mains_power_state_t:
  TK_MAINS_POWER_PRESENT
  TK_MAINS_POWER_NOT_PRESENT
  TK_MAINS_POWER_FAULT

tk_pump_relay_state_t:
  TK_PUMP_RELAY_ACTIVE
  TK_PUMP_RELAY_INACTIVE
  TK_PUMP_RELAY_FAULT
```

Fault details are part of the MQTT/backend contract direction, but the C telemetry struct is still minimal.

## Telemetry logic

The current dependency-free telemetry logic is:

```c
int tk_should_publish_telemetry(
    const tk_telemetry_t *last_published,
    const tk_telemetry_t *current,
    uint64_t time_since_last_publish_ms
);
```

The function returns true when the current telemetry sample should be published.

Current behavior:

* returns false if the current sample pointer is missing
* returns true if there is no previous published sample
* returns true if `mains_power` changed
* returns true if `pump_relay` changed
* returns true if the publish timeout elapsed
* otherwise returns false

This logic belongs in `tilekoumanto_logic`.

FreeRTOS task scheduling, tick conversion, platform callbacks, and logging belong in the telemetry task/runtime adapter.

## Status task

The shared core owns a simple status task.

The status task periodically calls the target-provided `status_led_toggle` callback when available.

The status task is currently a runtime heartbeat. It is not product telemetry and does not represent pump, mains, MQTT, LTE, or backend state.

Targets without a status LED may leave the callback unset.

## FreeRTOS ownership

The shared core owns the FreeRTOS application runtime.

Core runtime responsibilities include:

* task creation
* scheduler start
* task loop timing
* periodic telemetry processing
* status heartbeat task execution

The target provides the FreeRTOS port, configuration, and build support needed for the target build.

Current direction:

```text
FreeRTOS runtime model: core-owned
FreeRTOS target config: target-owned
FreeRTOS port selection: target-owned
hardware/platform setup: target-owned
dependency-free decision logic: tilekoumanto_logic
```

Each target owns its concrete `FreeRTOSConfig.h`.

Each target `FreeRTOSConfig.h` includes the core-owned `FreeRTOSConfig_core.h`.

The core-owned FreeRTOS configuration header defines runtime policy required by the shared core, such as static allocation support, idle-hook usage, stack-overflow checking, and required task delay APIs.

Target-owned FreeRTOS values include CPU clock, tick rate, interrupt priority settings, stack sizing, and port-specific configuration.

Position-independent code is target/build-specific. It must not be forced by the shared core.

The Python simulator may use position-independent code for shared-library or FFI use.

Bare-metal STM32 firmware should not use position-independent code by default.

## Python simulator target

The Python simulator is a real target, not a separate implementation of device logic.

It builds the shared C core as a target-local shared library and loads it through `ctypes`.

The simulator currently:

* provides the platform callbacks
* simulates mains power and pump relay readings
* publishes telemetry over MQTT/mTLS
* supports clean shutdown through the `should_stop` callback

The simulator is useful for validating:

* the shared C core runtime
* FreeRTOS POSIX execution
* telemetry publish decision behavior
* MQTT/mTLS publishing
* backend ingestion contract compatibility

The simulator should eventually support deterministic scenarios in addition to random input generation.

Possible scenario types:

* steady state
* mains power loss
* mains power restore
* pump relay active
* pump relay inactive
* unreadable mains input
* unreadable pump relay input

## STM32 target

The STM32 target scaffold exists under:

```text
device/targets/stm32
```

The current STM32 development target is:

```text
NUCLEO-F446RE
```

The STM32 target now has a minimal shared-core runtime integration on the NUCLEO-F446RE.

The current STM32 target:

* initializes basic board clock
* initializes LD2
* initializes USART2 logging
* provides a minimal `tk_platform_t`
* calls `tk_core_run(&platform)`
* runs the shared core FreeRTOS runtime
* runs the core-owned telemetry task
* runs the core-owned status task
* returns a fixed telemetry sample from a bring-up telemetry callback
* logs publish activity through a publish stub
* toggles LD2 through the status LED callback

The STM32 target currently uses the ARM Cortex-M4F FreeRTOS port.

The STM32 target has a target-owned `FreeRTOSConfig.h` and includes the shared `FreeRTOSConfig_core.h`.

The STM32 target still does not:

* read real field inputs
* debounce input signals
* publish telemetry over MQTT
* use LTE or a modem
* load device identity
* handle certificate material

## Connectivity direction

Initial connectivity work is cellular-oriented.

The current cellular module direction is:

```text
M5Stack U111-B
```

The long-term connectivity direction should remain pluggable.

Possible future connectivity choices include:

* cellular
* Wi-Fi
* Ethernet

The core should not care which connectivity mechanism is used. Connectivity-specific implementation belongs behind the target/platform publish callback.

## MQTT and identity direction

Device identity is certificate-backed.

The MQTT topic carries the device UUID:

```text
devices/<device_uuid>/pump/telemetry
```

The Python simulator derives certificate paths from:

```text
certs/devices/<uuid>/
├── <uuid>.crt
├── <uuid>.key
└── ca.crt
```

The simulator environment file provides:

```env
TK_DEVICE_UUID=<uuid>
TK_MQTT_HOST=<host>
TK_MQTT_PORT=8883
TK_CERTS_ROOT=/path/to/certs
```

The device UUID is not currently included in MQTT message metadata.

The backend obtains the device UUID from the MQTT topic.

## Current backend-facing telemetry direction

The current backend-facing telemetry model uses readable states and a fault list.

This document records the device-side intent only.

The authoritative backend validation contract lives in:

```text
backend/django/devices/contracts/mqtt.py
```

The Python simulator publisher should stay aligned with that contract:

```text
device/targets/python-sim/app/publisher.py
```

## Device-side testing

The device core now has an initial CTest baseline.

The current C test setup is intentionally small.

Current direction:

```text
tilekoumanto_logic
  Unit-testable C logic.

tilekoumanto_core
  Runtime integration.
  Not the first target for unit tests.
```

The first device-side tests should focus on dependency-free logic.

Tests should not require:

* FreeRTOS scheduling
* STM32 hardware
* Python runtime
* MQTT
* certificates
* Docker services
* backend services

Runtime, target, and full-stack integration tests can be added later when the relevant behavior is stable enough to protect.

## Open implementation areas

Known device-subsystem follow-ups:

* clean up STM32 bring-up logging
* add deterministic Python simulator scenarios
* add simulator fault generation
* decide how fault state should enter the C telemetry model
* implement STM32 real input reading and debouncing
* implement real modem/network publishing
* define how device identity is stored on embedded hardware
* define how firmware logs are handled on STM32
