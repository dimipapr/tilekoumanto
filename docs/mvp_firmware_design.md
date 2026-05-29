# Firmware Design: MVP Monitoring Device

## 1. Purpose

This document defines the firmware design for the MVP Tilekoumanto device.

The MVP firmware is responsible for secure telemetry publishing only.

It is not responsible for remote control, command execution, pump start/stop actions, edge automation, alerting, RBAC, user authentication, or advanced telemetry.

The MVP device is a secure telemetry publisher.

## 2. Primary Design Goal

The firmware must support two execution targets:

```text
STM32 target
  real hardware, real GPIO, embedded networking/TLS/MQTT

Simulator target
  simulated inputs, host-side networking/TLS/MQTT, development/test harness
```

The goal is to keep shared device behavior in one place and make target-specific behavior pluggable.

Shared behavior should include:

* telemetry publication policy
* firmware task structure
* polling cadence
* heartbeat behavior
* telemetry workflow
* message encoding rules
* runtime service orchestration

Target-specific behavior should include:

* GPIO/input implementation
* monotonic time source
* device Unix time source
* MQTT/TLS implementation details
* certificate/credential loading
* logging sink
* target startup and wiring

The simulator should behave as much like the STM32 firmware as practical. It should not become a separate reimplementation of device behavior.

## 3. MVP Firmware Goal

A provisioned device shall:

1. authenticate to the MQTT broker using mTLS
2. observe field inputs
3. create telemetry samples
4. publish telemetry over MQTT
5. publish again when observed values change
6. publish heartbeat telemetry when values do not change

The backend validates, stores, and exposes the latest accepted state.

## 4. Pluggability Principle

Shared firmware code must depend on interfaces, not on a specific target.

The STM32 target and simulator target provide different implementations of the same platform contract.

Conceptually:

```text
shared firmware
      ↓
platform API
   ↙       ↘
STM32     simulator
```

The shared firmware should not need to know whether inputs come from GPIO pins or simulated state.

The shared firmware should not need to know whether MQTT is implemented through an embedded stack or a host-side simulator transport.

The target is selected by the build and app wiring, not by spreading target checks throughout the firmware logic.

## 5. Firmware Responsibilities

For MVP, firmware is responsible for:

* holding or accessing provisioned device credentials
* establishing MQTT/mTLS communication
* observing `mains_present`
* observing `pump_relay_active`
* obtaining `device_unix_time_ms`
* deciding when telemetry should be published
* formatting the MVP telemetry payload
* publishing telemetry to the configured MQTT topic
* maintaining enough runtime behavior to retry/reconnect as needed

Firmware is not responsible for:

* user authentication
* user authorization
* user-device assignment
* backend validation policy
* storing long-term history
* deciding online/offline status
* receiving commands
* starting the pump
* stopping the pump
* command acknowledgement
* alerting
* pressure telemetry
* voltage/current telemetry

## 6. Telemetry Contract

The MVP telemetry message contains metadata and payload.

Device identity is not included in the payload. It is derived from the authenticated MQTT/mTLS connection, broker authorization, and topic context.

Example:

```json
{
  "metadata": {
    "device_unix_time_ms": 1234567890123
  },
  "payload": {
    "mains_present": true,
    "pump_relay_active": false
  }
}
```

### 6.1 Metadata

`device_unix_time_ms`

Integer.

Device-provided Unix time in milliseconds.

Represents the device wall-clock time when the telemetry sample was created.

### 6.2 Payload

`mains_present`

Boolean.

Represents observed mains presence at the monitored point.

`pump_relay_active`

Boolean.

Represents observed pump relay/contact active state.

## 7. Telemetry Publication Rules

The device should publish telemetry when:

* the first valid observation is available after boot/connect
* `mains_present` changes
* `pump_relay_active` changes
* the heartbeat interval elapses, even if values have not changed

## 8. Time Model

Firmware uses two different kinds of time.

### 8.1 Monotonic Time

Monotonic time is used internally for intervals.

Uses:

* polling interval
* heartbeat interval
* reconnect backoff
* future debounce windows
* future timeouts

Representation:

```c
uint32_t monotonic_time_ms;
```

Monotonic time may wrap around. Elapsed time checks must use unsigned subtraction:

```c
(uint32_t)(current_time_ms - previous_time_ms) >= interval_ms
```

### 8.2 Device Unix Time

Device Unix time is used in telemetry metadata.

Representation:

```c
uint64_t device_unix_time_ms;
```

Uses:

* telemetry event timestamp
* debugging
* ordering device-side samples

Device Unix time must not be used for internal heartbeat scheduling.

## 9. Firmware Architecture

The firmware is organized around a top-level split between shared code and target-specific code.

```text
device/
├── shared/
└── target/
````

`shared/` contains code used by both execution targets:

```text
STM32 target
Linux simulator target
```

`target/` contains code that depends on a specific execution environment.

The rule is simple:

> If code can be shared by both targets, it belongs under `shared/`.
> If code cannot be shared by both targets, it belongs under `target/`.

The exact internal structure of `shared/` and `target/` may evolve during implementation.

---

### 9.1 Shared Code

Shared code is device behavior that is common to both targets.

Shared code must not depend directly on STM32-specific or Linux-specific implementation details.

Shared code may depend on target-provided interfaces.

Shared code should not know whether it is running on the STM32 target or the Linux simulator target.

---

### 9.2 Target-Specific Code

Target-specific code contains target reality.

```text
device/target/
├── stm32/
└── linux/
```

Each target provides its own startup, wiring, configuration, and platform implementations.

Target code is allowed to know which target is being built.

---

### 9.3 Platform Contract

Shared code depends on a platform contract, not directly on STM32 or Linux code.

Conceptually:

```text
shared code
     ↓
platform contract
  ↙       ↘
stm32     linux
```

The platform contract allows shared code to request target-provided capabilities without knowing how the target implements them.

The platform contract should be defined by what shared code needs, not by what a target happens to provide.

---

### 9.4 App Boundary

Target startup and wiring belong under `target/`.

The target app selects and wires the target implementation used by shared code.

Shared code should not contain scattered target-selection logic.

Target selection should happen through:

* build configuration
* target app wiring
* platform implementation selection

---

### 9.5 Design Rule

Shared code contains common device behavior.

Target code contains target-specific reality.

If code can be shared, it is shared.

If code cannot be shared, it is target-specific.

When in doubt, start target-specific. Move code into `shared/` only when both targets need the same behavior.

## 10. First Firmware Slice

The first firmware slice proves telemetry publication behavior without adding remote control or advanced device logic.

The goal is:

```text
observed inputs
+ monotonic time
+ previous publication state
→ publish telemetry or do not publish
````

This slice exists to prove the smallest shared behavior required by the MVP.

---

### 10.1 Inputs

The first slice uses the MVP observed inputs:

```c
typedef struct {
    bool mains_present;
    bool pump_relay_active;
} tk_observed_inputs_t;
```

These values represent physical observations provided by the target implementation.

They are not desired state.

---

### 10.2 State

The first slice keeps only the state needed to decide telemetry publication:

```c
typedef struct {
    bool has_published_telemetry;
    tk_observed_inputs_t last_published_inputs;
    uint32_t last_telemetry_publish_ms;
    uint32_t telemetry_publish_interval_ms;
} tk_telemetry_publication_state_t;
```

---

### 10.3 Output

The first slice returns only a publication decision:

```c
typedef struct {
    bool should_publish_telemetry;
} tk_telemetry_publication_result_t;
```

The slice does not return telemetry content because, for MVP, the telemetry content is the current observed input values already held by the caller.

---

### 10.4 Rules

Telemetry should be published when:

* telemetry has never been published before
* `mains_present` changed since the last published telemetry
* `pump_relay_active` changed since the last published telemetry
* the heartbeat interval elapsed since the last published telemetry

Telemetry should not be published when:

* the observed values have not changed
* the heartbeat interval has not elapsed
* telemetry has already been published at least once

---

### 10.5 Time Rule

The publication decision uses monotonic time only.

Elapsed time must be calculated using unsigned subtraction:

```c
(uint32_t)(current_time_ms - previous_time_ms) >= interval_ms
```

This allows normal `uint32_t` wraparound.

---

### 10.6 Non-Responsibilities

This slice does not:

* read GPIO
* publish MQTT
* format JSON
* obtain device Unix time
* manage certificates
* connect or reconnect to the broker
* start FreeRTOS tasks
* decide online/offline status
* receive commands
* control outputs