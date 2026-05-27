# Working Notes

mqtt device outbound message
```json
{
  "metadata": {
    "device_uuid": "string (UUIDv4)",
    "timestamp": "integer (Unix Epoch seconds)",
  },
  "payload":"json depending on message type encoded into topic"
}
```

mqtt device telemetry topic and payload
- topic
```
devices/device-uuid/pump/telemetry
```
- payload
```json
{
  "mains_present":"bool",
  "relay_active":"bool",
}

```

## STATUS: Tilekoumanto IoT Backend

**Current State:** Green / Running. Data is actively flowing from the edge to the database.

### The Pipeline

1. **Edge** publishes JSON telemetry.
2. **Mosquitto** authenticates via mTLS (`mqtt-dev.tilekoumanto.gr`).
3. **Django Catcher** verifies UUID and saves raw payload to **PostgreSQL**.

### Critical Paths & Config

* **Compose File:** `backend/compose.yml`
* **Network:** Internal Docker network (postgres, mosquitto, django-web).
* **Cert Volumes:** Mounted at `/mosquitto/certs/` and `/certs/devices/` (explicitly outside `/app` to prevent nested volume ghost folders).
* **Environment:** Requires `.env` with DB creds and Django secret key.

### The Data Layer (Stateless Models)

* **`Device`**: Stores `uuid` and `last_seen` timestamp.
* *Note: State logic (`is_online`) was deliberately kept out of the model to prevent server polling. The frontend will derive state locally using `last_seen`.*


* **`DeviceMessageRaw`**: High-speed ledger. Stores the unparsed JSON `payload` and edge `device_timestamp`.

### Daily Commands

**Start Stack:**

```bash
sudo docker compose up -d

```

**Provision New Devices:** (Reads `manifest.json` and adds to DB)

```bash
sudo docker compose exec django-web python manage.py load_manifest

```

**Run the Listener:** (Starts the background worker)

```bash
sudo docker compose exec django-web python manage.py mqtt_catcher

```



### Firmware Architecture Specification

#### 1. Hardware Target

* **MCU:** STM32 (Development target: Nucleo board).
* **Network Interface:** Cellular Modem via UART (AT Commands). Future expansion planned for Wi-Fi and LAN.
* **Execution Environment:** FreeRTOS.
#### 2. Software Abstraction Layer (HAL)

The firmware strictly separates business logic from hardware dependencies to enable seamless testing and cross-platform compilation.

* **Platform-Independent Core (`tile_core.c`):** Contains all operational logic, state management, and MQTT payload formatting. It has zero knowledge of the underlying OS or hardware.
* **Hardware Abstraction Layer (`tile_hal.h`):** The defined contract of inputs/outputs the core requires.
* **Platform-Specific Implementations:** Connects the HAL to the physical world (e.g., `hal_stm32.c` implementing GPIO reads and FreeRTOS mutexes).

#### 3. Simulation & Testing Strategy (FFI Bridge)

To validate both the firmware logic and the backend integration, the project utilizes a Foreign Function Interface (FFI) Python bridge.

* The core logic (`tile_core.c`) is compiled as a shared library (`.so` / `.dll`).
* A Python testing harness (`device/simulator.py`) loads the library using `ctypes`.
* Python injects mock hardware functions into the C core, allowing the simulator to script complex physical environments (e.g., dropping water pressure) and network conditions while executing the exact C production logic.

### Implementation Slice 1: Basic Telemetry Loop

**Objective:** Establish the core timing and state-reporting loop in the hardware-agnostic architecture.

**Behavioral Requirements:**
* **Fast Loop (1 Hz):** The core must read the physical inputs (Mains Present, Relay Active) from the hardware layer once every second.
* **Slow Loop (1/60 Hz):** The core must format and publish a JSON telemetry message every 60 seconds as a standard heartbeat.
* **Event-Driven Update:** If the physical state changes during the 1 Hz polling intervals, the core must immediately publish the new state, overriding the 60-second heartbeat timer.

## Work Log: 2026-05-27
- **Firmware Core (`tk_core.c`):**
    - Completed basic telemetry loop: implemented 1 Hz polling, event-driven publishing, and 60s heartbeat logic.
    - Synchronized `TkTelemetry` struct in C with Python bindings via `ctypes`; resolved memory access and initialization issues.
    - Implemented HAL callbacks (telemetry, mqtt, time, lock, unlock) to bridge Python simulator to C logic.
- **Simulator (`simulator.py`):**
    - Integrated `paho-mqtt` with mTLS for production-grade secure transport.
    - Implemented path resolution using `pathlib` for robust cross-platform file anchoring.
    - Configured environment variables (UUID/Host) via system `os.environ` (manual injection).
- **Integration & Debugging:**
    - Verified end-to-end telemetry pipeline: Simulator → mTLS → Mosquitto → Django Catcher → PostgreSQL.
    - Debugged Django ingestion: Corrected timestamp parsing logic (converted incoming milliseconds to seconds) to resolve date serialization errors.
    - Validated FFI bridge: Confirmed accurate structural data transfer between Python memory space and C-core.

## Firmware Architectural Blueprint

We are building a production-grade, asynchronous embedded core inside a Linux simulation environment before moving to physical STM32 hardware.

### 1. The Tech Stack

* **Core:** Pure C (`libtk_core.so`) compiled via CMake with a strict **Debug Profile** (`-g -O0`).
* **OS Engine:** FreeRTOS POSIX port (running on Linux pthreads) with a hard-capped static heap size of **128 KB**.
* **Test Harness:** Python script communicating with the C core via `ctypes` FFI.
* **Backend:** Django Catcher syncing data to a PostgreSQL ledger via Mosquitto MQTT.

### 2. The State Pattern (Anti-Over-Engineering)

We rejected complex, event-driven queues because they are fragile during boot cycles and prone to flooding. Instead, we chose a **Hybrid Absolute-State Loop**:

* **The Shared Basket:** A single, mutex-protected C struct holds the absolute truth of the physical pins (e.g., `mains_present = true`).
* **The Input (Python):** The simulator updates the basket via FFI and triggers a lightweight, non-blocking **Task Notification** (`xTaskNotifyGive`).
* **The Processor (FreeRTOS Task):** The main logic task sleeps at **0% CPU** until notified. It wakes up instantly, copies the basket, and runs the state machine.
* **The Safety Heartbeat:** If no input happens for say 5 seconds, the task wakes up anyway to verify the state, ensuring the system self-heals if it boots up weirdly.

---

## Current Workspace Layout

```text
tilekoumanto/
├── backend/               # Django + Postgres stack (Debug logging enabled)
└── device/
    ├── src/
    │   ├── tk_core.c      # Production business logic (Hybrid loop lives here)
    │   └── tk_hal.h       # Hardware interface contract
    ├── third_party/
    │   └── FreeRTOS/      # FreeRTOS source + POSIX port
    └── CMakeLists.txt     # Build engine (Configured for Debug profile)

```

We are completely decoupled. The main execution loops never block on slow disk or network I/O.

Ready to pull down the FreeRTOS dependencies and write the config file? Give the word and we take the first step.


Our immediate objective is to transition from this single-threaded, time-delta dependent polling model into a multi-threaded, deterministic RTOS core on Linux, without changing the core business logic or the output payload format.

Let's define the explicit target state for this slice:

---

## The Target Operational Slice

### 1. Architectural Pivot

* Eliminate `tk_core_tick()` entirely. Control of execution will be handed over to the FreeRTOS scheduler permanently via a new startup entry point (e.g., `tk_core_start()`).
* Replace host-driven loop slicing with native FreeRTOS context switching managed by the Linux POSIX pthread portable layer.

### 2. Thread-Safe Shared State

* Eliminate the use of external, host-provided `lock()` and `unlock()` callbacks for data fetching.
* The FFI layer (Python) will inject state directly into an internal, static C struct protected by a native FreeRTOS Mutex.

### 3. The Hybrid Processing Loop (Single Task)

We will introduce a single FreeRTOS worker task (`vLogicTask`) that replaces the current time-arithmetic logic with a hybrid state-notification + timeout loop:

* **The Notification Path:** The task blocks completely at **0% CPU** utilizing `ulTaskNotifyTake()`. When Python updates the telemetry via an FFI callback, it signals this task to wake up and process the changes immediately.
* **The Heartbeat Path:** The task blocker uses a strict maximum timeout of **5000ms**. If no state updates occur within 5 seconds, the timeout expires, forcing the task to wake up and execute a mandatory telemetry sync.
* **The Output Condition:** The task compares the current state against the cached `last_published_state`. It formats and fires the identical JSON string via `mqtt_publish()` **only** if a physical pin value has flipped OR the 5-second safety timeout has been hit.

---

This defines the exact scope of what we want to build right now. Do you accept this target definition, or should we adjust the mechanics before moving to the compilation setup?