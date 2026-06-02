# Device Subsystem Design

## Purpose

This document captures the current design direction for the device-side subsystem.

The device subsystem is responsible for observing field-side pump signals, turning those observations into device telemetry, and publishing that telemetry to the backend.

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

The NUCLEO-F446RE is used as a capable STM32 development target for exploration. The final product hardware may be reduced later once the firmware and product requirements are clearer.

## Current signal model

The first field signal source is an auxiliary/dry contact from external electrical equipment.

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

The telemetry contract includes a fault list, but fault publishing is still minimal.

The authoritative fault schema lives in the backend MQTT contract:

```text
backend/django/devices/contracts/mqtt.py
```

The current MVP does not directly measure per-phase voltage, per-phase current, imbalance, dry-run risk, or bearing wear.

Those are possible later extensions once the basic device-to-backend telemetry path is stable.

## Telemetry behavior

The device publishes telemetry when any of the following is true:

* there is no previous successful publish
* `mains_power` changed
* `pump_relay` changed
* the publish timeout elapsed

The device does not publish every input sample if the state is unchanged and the publish timeout has not elapsed.

The current telemetry loop period is an implementation detail and may change as the device runtime matures.

## Runtime architecture

The device-side implementation is split into a shared C core and pluggable targets.

```text
device/
├── core/
└── targets/
    ├── python-sim/
    └── stm32/
```

The shared core owns task creation and scheduler startup.

The current application task graph contains one task:

```text
telemetry task
```

Targets own platform implementation and provide callbacks to the core.

The core is intended to be the shared device brain. Targets should not duplicate device decision logic.

## Shared core responsibilities

The shared C core is responsible for:

* owning the FreeRTOS runtime model
* exposing `tk_core_run()` as the target handoff entrypoint
* creating core-owned tasks
* running the telemetry loop
* reading telemetry through the platform interface
* deciding whether telemetry should be published
* remembering the last successfully published telemetry sample
* calling target-provided publish behavior

The core must not depend on:

* STM32 LL APIs
* GPIO registers
* UART registers
* modem AT commands
* Python runtime details
* simulator-specific behavior
* backend database details

## Shared core logic split

The shared core now separates dependency-free logic from runtime/task integration.

Current C layering:
```text
tilekoumanto_logic
  Pure C logic with no FreeRTOS dependency.

tilekoumanto_core
  Runtime-oriented core library.
  Links `tilekoumanto_logic` and FreeRTOS.
```
Current source split:
device/core/src/tk_telemetry.c
  Telemetry decision logic.

device/core/src/tk_telemetry_task.c
  FreeRTOS telemetry task/runtime adapter.

## Target responsibilities

Targets are responsible for:

* hardware or simulator initialization
* implementing the platform callback table
* providing wall-clock Unix time
* reading or simulating telemetry inputs
* publishing telemetry through the target transport
* handling target-specific shutdown behavior when applicable

The target initializes enough platform state for the core to run, then calls:

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
} tk_platform_t;
```

Current callback meanings:

```text
log
  Target-provided logging sink.

unix_time_ms
  Target-provided wall-clock timestamp used for telemetry timestamps.

read_telemetry
  Target-provided telemetry input reader.
  The target fills a tk_telemetry_t sample.

publish_telemetry
  Target-provided publish operation.
  The core decides when to publish; the target decides how to publish.

should_stop
  Target-provided stop request.
  Mainly used by simulator targets for clean shutdown.
  Embedded targets can return 0 forever.
```

## Shared telemetry type

The current shared telemetry sample contains:

```text
mains_power
pump_relay
unix_time_ms
```

Current conceptual shape:

```c
typedef struct {
    tk_mains_power_state_t mains_power;
    tk_pump_relay_state_t pump_relay;
    uint64_t unix_time_ms;
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

Fault details are currently part of the MQTT/backend contract direction, but the C telemetry struct is still minimal.

## FreeRTOS ownership

The shared core owns the FreeRTOS application runtime.

Core responsibilities include:

* task creation
* scheduler start
* task loop timing
* periodic telemetry processing

The target provides the FreeRTOS port/config/build support needed for the target build, but the application task graph belongs to core.

Current direction:

```text
FreeRTOS runtime model: core-owned
hardware/platform setup: target-owned
```

## Python simulator target

The Python simulator is a real target, not a separate implementation of device logic.

It builds the shared C core as a target-local shared library and loads it through `ctypes`.

The simulator currently:

* provides the platform callbacks
* simulates mains power and pump relay readings
* publishes telemetry over MQTT/mTLS
* supports clean shutdown through the `should_stop` callback

The simulator is currently useful for validating:

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

The STM32 target currently has an initial CMake/LL-driver scaffold and a first firmware build path.

Runtime integration with the shared C core is still pending.

The STM32 target should eventually:

* initialize board hardware
* initialize required peripherals
* implement `tk_platform_t`
* read real device inputs
* publish telemetry through the selected transport
* call `tk_core_run(&platform)`
* let the shared core own the FreeRTOS application runtime

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

The device UUID is not currently included in MQTT message metadata. The backend obtains the device UUID from the topic.

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

## Open implementation areas

Known device-subsystem follow-ups:

* add deterministic Python simulator scenarios
* add simulator fault generation
* decide how fault state should enter the C telemetry model
* cleanly integrate the shared core into the STM32 target
* implement STM32 `tk_platform_t`
* implement real input reading and debouncing
* implement real modem/network publishing
* define how device identity is stored on embedded hardware
* define how firmware logs are handled on STM32

