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