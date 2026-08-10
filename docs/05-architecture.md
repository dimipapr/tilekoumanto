# Architecture

## 1. System purpose

Tilekoumanto monitors an agricultural irrigation pump installation remotely.

For the MVP, the system only needs to expose two field states:

- mains power state
- pump relay state

The purpose of the architecture is to move these states from the field installation to a backend system where they can be stored and viewed through an API.

## 2. MVP architecture

The MVP architecture consists of:

- external relay contacts for electrical state detection
- an STM32-based field proccessor
- an ESP32 communications proccessor
- a UART link between STM32 and ESP32
- wifi connectivity from the ESP32
- MQTT over mutual TLS
- Mosquitto as the MQTT broker
- Django as the backend/API
- PostgreSQL as the database
- Caddy as the HTTP entrypoint/reverse proxy
- Docker Compose for local deployment

For the MVP, the farmer-facing interface is the API. No separate web dashboard or mobile application is included yet.

## 3. Field device

The field device uses two processors with separate responsibilities.

### 3.1 STM32 field processor

The STM32:

- reads the mains-power and pump-relay input signals
- creates the backend-facing telemetry payload
- sends newline-delimited telemetry to the ESP32 over UART

The STM32 does not connect directly to the MQTT broker.

### 3.2 ESP32 communications processor

The ESP32:

- receives newline-delimited telemetry from the STM32
- connects to the network over Wi-Fi
- publishes telemetry to Mosquitto using MQTT over mutual TLS

The ESP32 owns network communication. It does not own field-input interpretation or telemetry generation.

## 4. Field inputs

### 4.1 Mains power state

Mains power state is detected through an external voltage-monitoring relay contact.

The STM32 reads this contact as a discrete input and treats it as the field signal for whether mains power is available at the installation.

The device does not directly measure three-phase mains voltage.

### 4.2 Pump relay state

Pump state is detected through a relay contact from the pump control circuit.

The STM32 reads this contact as a discrete input and treats it as the field signal for whether the pump relay is active.

The device does not directly measure motor current or hydraulic behavior.

## 5. Communication

The system has three communication boundaries:

- STM32-to-ESP32 telemetry over UART
- ESP32-to-backend telemetry over MQTT/mTLS
- user-to-backend access over HTTPS

For the MVP, telemetry communication is one-way from the field device to the backend. Remote control is not included.

### 5.1 STM32-to-ESP32 link

The STM32 sends backend-facing JSON telemetry to the ESP32 over UART.

Messages are newline-delimited. The ESP32 reconstructs complete messages before forwarding them.

UART acknowledgements, delivery retries, and offline buffering are deferred.

### 5.2 ESP32-to-backend telemetry

The ESP32 publishes telemetry over Wi-Fi using MQTT with mutual TLS.

Current MVP telemetry path:

```text
STM32
→ UART
→ ESP32
→ Wi-Fi
→ MQTT over mTLS
→ Mosquitto
→ Django MQTT ingestion
→ PostgreSQL
```

### 5.3 MQTT transport security

Each field device uses a client certificate issued by the project-controlled certificate authority.

Mutual TLS provides:

* telemetry encryption in transit
* broker identity validation by the device
* device authentication by the broker
* rejection of unknown clients

### 5.4 User-facing HTTPS channel

Users access the backend API over HTTPS through Caddy.

Current MVP user-facing path:

```text
User / API client
→ HTTPS
→ Caddy
→ Django API
→ PostgreSQL
```

### 6. Backend

The backend consists of:

* Mosquitto for MQTT message handling
* Django for telemetry ingestion, application logic, and the API
* PostgreSQL for persistent storage
* Caddy as the HTTPS entrypoint and reverse proxy

Django validates incoming telemetry, stores the reported field state, and exposes the latest known state through the API.

### 7. Data storage

PostgreSQL stores:

* device identity
* backend receive time
* device-reported time when available
* mains power state
* pump relay state

The backend exposes the latest known state for a device through the API.

### 8. Farmer-facing interface

For the MVP, the farmer-facing interface is the API.

The API exposes:

* whether mains power is available
* whether the pump relay is active
* when the latest state was received

The existing /devices page is operator/developer tooling. It is not a separate farmer-facing dashboard.

### 9. Deployment

The current backend deployment target is a local machine using Docker Compose.

The local deployment includes:

* Caddy
* Django web API
* Django MQTT ingestion
* PostgreSQL
* Mosquitto

The backend stack is intended to be transferable to a VPS without a major architecture change.

### 10. MVP data flow

```text
Field installation
→ relay contacts
→ STM32
→ UART
→ ESP32
→ Wi-Fi
→ MQTT over mTLS
→ Mosquitto
→ Django ingestion
→ PostgreSQL
→ Django API
```

### 11. Deferred architecture

The following are not part of the current MVP architecture:

Ethernet connectivity
cellular connectivity
remote pump control
pressure monitoring
production credential provisioning and private-key protection

Detailed device hardware and software architecture is documented in 09-device-subsystem.md.