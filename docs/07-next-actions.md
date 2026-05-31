# Next Actions

## Current mode

Pivot toward MVP device behavior and firmware architecture.

The backend monitoring path is working well enough to support device-side work. A temporary Python simulator can publish randomized pump telemetry, and the latest state is visible through the API and `/devices` operator page.

## 1. Define MVP device behavior

Create a short device behavior document.

Suggested file:

```text
docs/10-device-behavior.md
````

Capture:

* what the MVP device is responsible for
* what the MVP device explicitly does not do
* expected runtime behavior
* current simulator behavior
* open questions before firmware work

Keep the MVP narrow:

* monitor mains power state
* monitor pump relay state
* publish telemetry periodically
* no remote start/stop
* no pressure monitoring
* no scheduling
* no automation

## 2. Sketch firmware architecture

After device behavior is written, sketch the first firmware architecture.

Cover only:

* device identity
* state reading
* telemetry payload creation
* MQTT publish loop
* reconnect behavior
* local/manual pump control staying independent

Do not design remote control yet.

## 3. Consider shared device core

The shared core should be limited to deterministic, platform-independent device logic.

Initial shared candidates:

- telemetry state representation
- MQTT topic construction
- telemetry payload construction
- simple state validation or normalization rules

Keep these outside the shared core:

- MQTT networking
- TLS setup
- certificate loading
- filesystem paths
- hardware/GPIO reads
- simulator randomization
- time source
- sleep/timers
- reconnect mechanics
- logging

The simulator and firmware should call the shared core, but own their platform-specific runtime loops.
