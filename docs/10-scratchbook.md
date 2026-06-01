# Scratchbook

## Purpose

This file is for temporary notes, rough ideas, exploratory design, and unsorted thoughts.

It is not a permanent source of truth.

Move stable facts into the appropriate documentation file when they are implemented, validated, or accepted as current project state.

## Current exploratory topic

Device subsystem design exploration.

The device firmware implementation has not started yet.

The notes below should not be treated as implemented behavior. Move them into `09-device-subsystem-design.md` only as device work begins and the design is confirmed by implementation.

---

# Device Subsystem Design Notes

## Working direction

The device subsystem is being explored as a separate field-side system.

The MVP device should monitor one agricultural pump installation and publish telemetry to the backend.

The device should not control the pump in the MVP.

## MVP monitoring scope

The MVP device should report:

- mains power state
- pump relay state

The MVP device should not include:

- remote start/stop control
- pressure monitoring
- water flow monitoring
- current measurement
- voltage measurement
- scheduling
- automation
- multi-field behavior
- farmer-facing UI logic

Local/manual pump control should remain independent from the device.

If the device loses power, crashes, disconnects, or is removed, the existing pump installation should continue behaving as before.

## Prototype hardware direction

Current prototype hardware direction:

- `NUCLEO-F446RE` as the MCU development board
- `M5Stack U111-B` as the cellular/NB-IoT communication module

Reasoning:

- `NUCLEO-F446RE` gives a capable STM32 development target with enough headroom for development, debugging, and exploration.
- The board may be over-specced for the final product.
- This is acceptable during MVP discovery.
- Once product behavior and firmware architecture are clearer, the MCU can be specced down for production.
- `M5Stack U111-B` provides the first concrete cellular/NB-IoT communication path.

This is a prototype exploration direction, not a final production hardware decision.

## Connectivity direction

The first prototype should use the `M5Stack U111-B` cellular/NB-IoT module.

Longer term, connectivity should remain pluggable.

Possible future transports:

- Wi-Fi
- Ethernet
- other cellular modules

For the MVP, only the `U111-B` path needs to be implemented.

Design direction:

- keep device behavior separate from transport-specific networking code
- avoid coupling pump monitoring logic directly to modem AT commands
- avoid assuming Wi-Fi or Ethernet in the first firmware architecture

Possible conceptual transport interface:

```text
connect()
disconnect()
is_connected()
publish(topic, payload)
````

This is only a design note, not an implementation requirement yet.

## Monitored inputs

The MVP device should read two digital inputs:

* mains power state
* pump relay state

Telemetry should be based on stable debounced input state, not raw GPIO edges.

## Mains power input

Current direction:

Use a relay contact from an external phase monitoring relay.

The firmware would treat this as a digital input.

Reading values:

```text
mains_power: present | not_present | fault
```

Reasoning:

* keeps the MCU isolated from direct mains measurement
* avoids designing phase-voltage measurement hardware in the MVP
* uses a purpose-built external device for phase monitoring
* keeps the firmware input model simple

Exact electrical polarity still needs to be confirmed during wiring.

Future direction:

Later versions may monitor each phase separately and may add voltage/current measurement for:

* phase imbalance detection
* voltage imbalance detection
* current imbalance detection
* early dry-run detection
* bearing wear or abnormal load detection
* richer pump health diagnostics

These are not part of the MVP.

## Pump relay input

Current direction:

Use an auxiliary contact on the pump contactor.

The firmware would treat this as a digital input.

Reading values:

```text
pump_relay: active | inactive | fault
```

Reasoning:

* directly reflects whether the pump contactor is engaged
* avoids inferring pump state from current draw in the MVP
* keeps the MCU isolated from the pump power circuit
* keeps the firmware input model simple

Exact electrical polarity still needs to be confirmed during wiring.

Future direction:

Later versions may compare contactor state against measured current, pressure, or flow to detect conditions such as:

* contactor active but pump not actually drawing current
* pump running dry
* blocked or disconnected irrigation line
* abnormal load
* mechanical wear

These are not part of the MVP.

## Reading state model

Current preferred model:

Use readable domain-specific states instead of booleans.

```text
mains_power: present | not_present | fault
pump_relay: active | inactive | fault
```

Fault details should be reported separately from reading values.

Potential rules:

* `payload.readings` is always present.
* `payload.readings.mains_power` is always present.
* `payload.readings.pump_relay` is always present.
* `payload.faults` is always present.
* `payload.faults` is an empty array when no faults are reported.
* If a reading is `fault`, at least one fault should reference that reading.
* Fault types should only be added when the device can actually detect them.

## Telemetry payload direction

Current preferred payload shape:

```json
{
  "meta": {
    "unix_time_ms": 1234567890000
  },
  "payload": {
    "readings": {
      "mains_power": "present",
      "pump_relay": "inactive"
    },
    "faults": []
  }
}
```

Example with a reading fault:

```json
{
  "meta": {
    "unix_time_ms": 1234567890000
  },
  "payload": {
    "readings": {
      "mains_power": "fault",
      "pump_relay": "active"
    },
    "faults": [
      {
        "target": "mains_power",
        "type": "input_unreadable"
      }
    ]
  }
}
```

Initial telemetry topic remains:

```text
devices/{device_uuid}/pump/telemetry
```

The device UUID remains the stable device identity.

## Current backend/simulator mismatch

The current development simulator and backend contract use the older boolean payload shape:

```json
{
  "meta": {
    "unix_time_ms": 1234567890000
  },
  "payload": {
    "mains_power_present": true,
    "pump_relay_active": true
  }
}
```

The device subsystem exploration currently prefers the newer reading/fault payload shape.

This is a known design-to-implementation mismatch.

Before firmware and backend work continue, the MQTT contract, simulator, ingestion code, storage model, and latest-state API may need to be updated or explicitly versioned.

Do not silently treat the new payload shape as implemented.

## Runtime direction

Current direction:

Use FreeRTOS from the start.

Reasoning:

* the firmware is expected to need concurrent responsibilities
* input sampling, debouncing, telemetry publishing, and connectivity management should not be forced into one blocking loop
* starting with FreeRTOS avoids a later architecture rewrite

## STM32 driver direction

Current direction:

Use STM32 LL drivers for firmware development.

Reasoning:

- LL drivers keep the firmware closer to the hardware than HAL
- LL drivers avoid some of the abstraction and overhead of HAL
- LL drivers still provide vendor-supported register-level helpers
- this fits a firmware architecture where hardware-specific code stays isolated from device behavior logic

The firmware should avoid mixing HAL and LL casually.

If generated CubeMX startup or configuration code is used, the project should be explicit about which parts are generated and which parts are application-owned.

The device behavior, telemetry construction, and transport abstraction should not depend directly on STM32 LL APIs.

## Possible firmware responsibilities

The MVP firmware may be responsible for:

- loading or knowing device identity
- initializing monitored inputs using STM32 LL drivers
- sampling digital inputs
- debouncing digital inputs
- maintaining current stable readings
- building telemetry payloads
- publishing telemetry through the current connectivity path
- reconnecting when connectivity is unavailable

The firmware should not be responsible for pump control in the MVP.

## Possible FreeRTOS responsibility split

Possible task groups:

```text
input sampling
├── read mains power input
├── read pump relay input
├── debounce raw input readings
└── publish stable input state internally

telemetry
├── build telemetry payload
├── publish periodically
└── publish after critical debounced state changes

connectivity
├── manage U111-B modem communication
├── establish network/MQTT connection
├── publish MQTT messages
└── reconnect when needed
```

The exact task split may change during implementation.

## Input sampling and debouncing

Digital inputs should be sampled at a relatively high frequency compared with telemetry publishing.

Raw input readings should be debounced before they are used as telemetry readings.

Telemetry should use stable debounced state.

Exact sampling frequency and debounce timing remain open implementation choices.

## Publishing behavior

The device should publish telemetry in two cases:

1. on a regular interval
2. when a critical debounced state change occurs

Periodic publishing acts as a heartbeat and latest-state refresh.

Event-triggered publishing makes important state changes visible quickly.

Critical MVP state changes:

* `mains_power` changes between `present`, `not_present`, and `fault`
* `pump_relay` changes between `active`, `inactive`, and `fault`

Event-triggered telemetry should be based on debounced state changes, not raw input edges.

The firmware should avoid repeated publishing caused by contact bounce or rapid flapping.

## Explicit non-MVP boundaries

Do not design or implement the following as part of the MVP device subsystem:

* remote pump start
* remote pump stop
* control relay output
* pressure sensor support
* flow sensor support
* voltage measurement
* current measurement
* electrical diagnostics
* dry-run detection
* bearing wear detection
* irrigation scheduling
* autonomous pump decisions
* multi-pump or multi-field logic

These may be considered later after the monitoring MVP is stable.

## Future directions

Possible future directions:

* Wi-Fi connectivity
* Ethernet connectivity
* alternative cellular modules
* per-phase voltage measurement
* per-phase current measurement
* pressure monitoring
* flow monitoring
* dry-run detection
* abnormal load detection
* pump health diagnostics
* remote start/stop with safety constraints

These are not part of the MVP.

## Open questions

* What exact GPIO pins will be used for the two digital inputs?
* What is the confirmed electrical polarity of each input?
* What input protection or isolation circuit will be used between relay contacts and MCU pins?
* What sampling frequency should be used for digital inputs?
* What debounce duration should be used?
* What regular telemetry interval should be used?
* Should event-triggered telemetry be rate-limited or coalesced?
* How will the Nucleo store or receive its device UUID?
* How will certificate or credential material be stored for the prototype?
* Can the U111-B support the required MQTT/TLS behavior directly, or will firmware need additional handling?
* Should `unix_time_ms` come from the device, modem/network time, or be omitted if unavailable?
* Should the backend continue treating receive time as the primary trusted timestamp?

## Device implementation plan

Current direction:

Start device work with the smallest possible hardware bring-up milestone.

First milestone:

```text
CMake-built blinky for NUCLEO-F446RE
```

Purpose:

* confirm the repository structure for device code
* confirm the ARM toolchain works
* confirm CMake can build the STM32 target
* confirm flashing/debugging through the onboard ST-LINK path
* confirm basic GPIO output using STM32 LL drivers
* avoid bringing in FreeRTOS, modem code, telemetry, or shared logic too early

## Device code organization direction

Use a top-level split under `device/`:

```text
device/
├── shared/
├── stm32/
└── simulator/
```

Purpose:

* `shared` contains deterministic platform-independent device logic.
* `stm32` contains the real STM32 firmware target.
* `simulator` contains the host-side development simulator.

Dependency direction:

```text
stm32     -> shared
simulator -> shared
shared    -> no platform-specific code
```

`shared` may include:

* telemetry reading types
* fault types
* state snapshot structures
* MQTT topic construction
* telemetry payload construction
* platform-independent debouncing logic
* state-change detection
* publish trigger decision logic

`shared` must not include:

* STM32 LL calls
* FreeRTOS APIs
* UART drivers
* modem AT commands
* TLS setup
* MQTT client implementation
* filesystem paths
* host randomization
* sleep/timers
* logging backend

`stm32` owns:

* CubeMX project files
* STM32 startup/configuration
* FreeRTOS integration
* LL driver usage
* GPIO reads
* UART communication with the U111-B
* modem runtime
* hardware timers
* task creation
* hardware-specific configuration

`simulator` owns:

* simulated input generation
* local device manifest/cert loading
* host MQTT client
* host timers/sleep
* randomized or scripted pump/mains states
* development CLI options

The simulator and STM32 firmware should both use `shared` for telemetry payload and topic construction so the device contract is not duplicated.

## STM32 generation and build direction

Use STM32CubeMX for STM32 configuration/reference generation, but use CMake as the project-owned build system.

CubeMX may be used for:

* `.ioc` configuration
* clock configuration
* startup files
* linker setup
* pin initialization
* GPIO setup
* UART setup
* FreeRTOS base integration
* interrupt configuration

CMake should be the build system used by the repository.

Do not rely on CubeMX-generated IDE/build projects as the main project build.

Use STM32 LL drivers as the preferred peripheral driver layer.

Guidelines:

* commit the CubeMX `.ioc` file
* keep generated/vendor STM32 files under a clearly named platform area
* keep application firmware code outside generated folders
* avoid editing generated/vendor files unless necessary
* document any required manual changes to generated/vendor files
* use CMake presets for host simulator and STM32 cross builds

Generated code initializes hardware.

Application-owned code implements behavior.

## Proposed STM32 directory shape

```text
device/
├── shared/
│   ├── include/
│   ├── src/
│   └── CMakeLists.txt
│
├── simulator/
│   ├── src/
│   └── CMakeLists.txt
│
└── stm32/
    ├── cubemx/
    │   └── tilekoumanto-f446re.ioc
    ├── platform/
    │   ├── Core/
    │   ├── Drivers/
    │   ├── Middlewares/
    │   ├── startup/
    │   └── linker/
    ├── app/
    │   ├── include/
    │   └── src/
    ├── CMakeLists.txt
    └── toolchain-arm-none-eabi.cmake
```

This structure may be adjusted during implementation, but the separation should remain clear:

```text
shared     = portable device logic
simulator  = host-side development target
stm32      = embedded target
platform   = generated/vendor STM32 scaffolding
app        = application-owned STM32 code
```

## Blinky milestone

Goal:

Build, flash, and run a minimal STM32 firmware that toggles the NUCLEO-F446RE user LED.

Scope:

* no FreeRTOS yet
* no modem
* no MQTT
* no telemetry
* no simulator changes
* no shared logic unless needed for build validation
* no input debouncing yet

Use:

* NUCLEO-F446RE
* STM32 LL drivers
* CMake
* ARM GCC toolchain
* onboard ST-LINK for flashing/debugging

Blinky should prove:

* the STM32 source layout works
* the selected startup file and linker script work
* the system clock is sufficient for a basic firmware
* GPIO output works
* the CMake build is understandable and repeatable

Suggested implementation steps:

1. Generate or create a minimal CubeMX configuration for NUCLEO-F446RE.
2. Configure the user LED pin as GPIO output.
3. Select STM32 LL drivers.
4. Generate reference files.
5. Move or copy required generated/vendor files into `device/stm32/platform/`.
6. Create a project-owned `device/stm32/CMakeLists.txt`.
7. Add minimal application code under `device/stm32/app/`.
8. Build with CMake and `arm-none-eabi-gcc`.
9. Flash with ST-LINK tooling.
10. Record the result in `06-implementation-log.md`.

Suggested first success criterion:

```text
The NUCLEO-F446RE user LED blinks from firmware built by the repository-owned CMake build.
```

## Blinky non-goals

Do not add these during the blinky milestone:

* FreeRTOS tasks
* input sampling
* debouncing
* telemetry payload code
* modem/U111-B code
* MQTT code
* certificate handling
* simulator refactor
* backend contract changes

Keep this milestone focused only on STM32 build and GPIO output bring-up.

## Follow-up milestones after blinky

Possible next steps after blinky succeeds:

1. Add FreeRTOS with one LED task.
2. Add two digital input reads using placeholder GPIO pins.
3. Add platform-independent debounce logic.
4. Move debounce/state logic into `device/shared`.
5. Add simulator target that uses the shared state model.
6. Add telemetry payload construction in `shared`.
7. Update backend/simulator contract to the new reading/fault payload shape.
8. Add UART communication with the U111-B.
9. Add modem bring-up.
10. Add MQTT publish path.

## Device code organization direction

Current direction:

Use a `core` and `targets` split under `device/`:

```text
device/
├── core/
└── targets/
    ├── stm32/
    └── simulator/
```

## Core/runtime ownership

Current direction:

The shared device core owns the application runtime.

Target-specific code initializes the hardware/platform, provides target implementations through a function-pointer interface, and passes control to core.

The target does not start the FreeRTOS scheduler directly.

Target responsibilities:

- MCU/board initialization
- clock setup
- GPIO/UART/peripheral setup
- FreeRTOS port/config/heap support
- target-specific driver implementations
- platform function table creation
- call `tk_core_run(&platform)`

Core responsibilities:

- validate/store the platform interface
- create FreeRTOS tasks
- create queues/timers/events
- own the device application task graph
- start the FreeRTOS scheduler
- run input sampling and telemetry scheduling logic
- call target-provided functions through the platform interface

Rule:

```text
target owns hardware/platform setup
core owns application runtime
```