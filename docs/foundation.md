# Software Requirements Specification (SRS): Tilekoumanto

## 1. Introduction

### 1.1 Product Purpose & Scope

Tilekoumanto is an IP-based remote monitoring and control system designed for irrigation pump installations. Its primary purpose is to provide users with accurate, up-to-date visibility into field operations without requiring physical presence.

The system operates in parallel with existing local controls. It shall not:

* Replace existing local controls.
* Alter the normal local operation of the machinery.
* Complicate manual operation of the installation.

### 1.2 Target Installations

The product specifically targets irrigation systems utilizing a START(NO)/STOP(NC) seal-in/latching circuit. The hardware is intended to retrofit around common pump control panels without necessitating a redesigned control cabinet.

### 1.3 Target Users & Roles

The system accommodates various user profiles, including farmers, landowners, irrigation operators, maintenance technicians, installers, and system administrators. Access is governed by a Role-Based Access Control (RBAC) model (e.g., Owner, Administrator, Operator, Viewer).

### 1.4 Safety & Security Goals

* **Safety Principle:** The system must strictly distinguish between user command intent and observed physical reality. It shall not hide uncertainty, present assumptions as facts, or confuse intended states with actual physical states. Unknown, stale, missing, invalid, or conflicting data must be explicitly visible to the user.
* **Security Principle:** Devices maintain independent identities separate from user accounts. The system supports a many-to-many relationship between users and devices, bound by specific permission levels, ensuring users only monitor and control authorized equipment.

---

## 2. System Architecture

### 2.1 Architecture Overview

Tilekoumanto utilizes two strictly separated communication paths that converge at the database level.

**Human Path:**

```text
Browser / PWA <—HTTPS/WSS—> Caddy <—http/ws—> Django Web-API <—> PostgreSQL

```

**Device Path:**

```text
Field device / simulator <—MQTTS—> Mosquitto <—> Django MQTT worker <—> PostgreSQL

```

### 2.2 Core Components

* **Field Installation:** The existing irrigation pump control panel combined with the Tilekoumanto-connected controller/device.
* **Tilekoumanto Device / Simulator:** Hardware or software that communicates with the backend. The simulator is used for validation without requiring production hardware.
* **Mosquitto:** The MQTT broker acting as the device transport security boundary. It handles TLS/mTLS and client certificate validation.
* **Django MQTT Worker:** A dedicated process (separate from the web process) that receives messages from Mosquitto, validates them, checks device state, and persists data.
* **PostgreSQL:** The shared system of record. It stores users, devices, relationships, lifecycles, logs, commands, and telemetry.
* **Django Web/API:** The business logic source of truth. It serves the application, enforces user permissions, and exposes device states to users.
* **Caddy:** The encrypted entry point acting as a reverse proxy for human/browser HTTPS traffic.

### 2.3 Architecture Principles

* **Local Process Resilience:** The physical irrigation process must never depend on Tilekoumanto being online. If any part of the Tilekoumanto stack fails or loses connection, the installation must remain fully operable via normal local controls.
* **Edge Protection:** The edge device must be capable of executing protective, deterministic, and auditable shutdown decisions locally without backend connectivity.
* **Observed State > Assumed State:** The system strictly treats user commands as intent, not fact. A "start command sent" does not equal "pump confirmed running." The system state relies exclusively on field-observed signals.
* **Traffic Separation:** Human traffic (HTTPS via Caddy) and device traffic (MQTT over mTLS via Mosquitto) are entirely isolated from one another at the ingress level.
* **Transport Security Ownership:** Mosquitto entirely owns MQTT transport security, enforcing the rule that the device certificate identity must match the device UUID used in MQTT topics.

---

## 3. Functional Requirements

### 3.1 Monitored Signals (Telemetry)

The system shall monitor and report the following physical field signals to establish the true state of the installation:

* **Mains Power:** Presence or absence of grid power.
* **Main Relay / Contactor:** Active or inactive state.
* **Water Pressure:** Presence or absence of pressure in the output line.
* **Power Quality:** Three-phase voltage readings.
* **Load Metrics:** Three-phase current draw readings.

### 3.2 Remote Control Capabilities

Authorized users shall have the ability to send discrete control commands to the field installation.

* **Supported Commands:** `Start` and `Stop`.
* **Command Lifecycle Tracking:** Remote control operations must be simple, clear, and safe. The system must explicitly track and display the lifecycle of a command:
1. What the user requested.
2. What command was dispatched to the broker.
3. What the edge device reported receiving.
4. What the resulting physical signals show (e.g., a "start" command is only considered successful when current draw and pressure are physically observed).



### 3.3 Edge Protection & Local Automation

The field device shall be capable of executing protective shutdown decisions locally, without requiring backend, app, or MQTT availability. These rules are designed strictly for equipment safety, not convenience automation.

The device shall trigger a safe local stop under the following deterministic conditions:

* Loss of a mains phase.
* Abnormal voltage or current measurements.
* Missing water pressure while the pump control circuit is active.
* Relay/contactor state is inconsistent with expected behavior.
* Detection of stale, missing, or invalid local sensor readings that compromise safe operation.

### 3.4 User Alerts & Notifications

The system shall proactively notify users of critical events, state changes, and fault conditions based on reported field telemetry.

* **Critical Fault Alerts:** Triggered immediately when edge protection rules engage to shut down the pump.
* **Connectivity Alerts:** Triggered when a field device fails to report within the expected heartbeat window, indicating the data is stale or the device is offline.
* **Power Events:** Notifications for the loss and subsequent restoration of mains power.
* **Operational Alerts (Configurable):** Optional notifications for standard state changes, such as pump starts and stops.

---

## 4. Data & State Management

### 4.1 Business Logic & Source of Truth

* **Django Backend:** Acts as the definitive source of truth for business rules, managing users, device lifecycles, permissions, and message validation.
* **PostgreSQL:** Serves as the central persistent database, storing configurations, relationships, communication logs, and historical telemetry.

### 4.2 Device State vs. Command Handling

* **Device State:** The system's view of the installation is derived strictly from recently reported field telemetry (mains presence, relay state, voltage/current levels, and last communication timestamp).
* **Command Tracking:** User commands are stored as *intent* requests. A command is only marked as successful when subsequent telemetry proves the physical change occurred.

### 4.3 Historical Data Logging

The system retains an auditable history for troubleshooting and operational review.

* **Events:** Pump start/stop cycles, power loss/restoration, and abnormal fault conditions.
* **Telemetry Trends:** Water pressure behavior, voltage fluctuations, and current consumption over time.
* **System Activity:** Remote commands issued, resulting field outcomes, communication events, and data validation failures.

### 4.4 User-Device Access

* **Many-to-Many Relationship:** A single user can access multiple devices, and a single device can be accessible by multiple users.
* **Role Assignments:** The user-device relationship includes a specific permission level:
* **Administrator/Owner:** Full visibility, remote control, and the ability to assign other users to the device.
* **Operator:** Full visibility and remote control execution.
* **Viewer:** Read-only access to current state and history.



---

## 5. Minimum Viable Product (MVP)

### 5.1 MVP Goal

The MVP proves the secure monitoring path end to end.

A provisioned device can authenticate with the MQTT broker using its device certificate, publish field telemetry over MQTT/mTLS, and have that telemetry validated, stored, and exposed by the backend.

A user can view the latest accepted device state over HTTPS.

For MVP, user-facing security means HTTPS transport security only. User authentication, session security, authorization checks, RBAC, and user-device assignment are intentionally excluded.

The MVP device is a secure telemetry publisher, not a remote controller.

---

### 5.2 MVP Scope: Included

#### Device Provisioning

The system supports provisioned device identity.

A device has:

* a stable device identity
* device credentials
* a backend-recognized device record or manifest entry

The device does not authenticate as a human user.

---

#### Secure Device Communication

The device communicates with the system using MQTT over mTLS.

The MVP includes:

* device certificate identity
* broker-side mTLS authentication
* broker-side authorization/ACLs
* MQTT telemetry publishing
* backend-side message validation

The backend must only accept telemetry from known and valid device identities.

Device identity is derived from the authenticated MQTT/mTLS connection, broker authorization, and topic context. The telemetry payload does not self-declare device identity.

---

#### Secure User Transport

The user-facing monitoring view is served over HTTPS via Caddy.

For MVP, HTTPS protects transport between the user and the backend.

The MVP does not include user login, sessions, authorization rules, or per-user device access control.

---

#### Telemetry Ingestion

The device publishes field telemetry.

MVP telemetry includes:

```text
device_unix_time_ms
mains_present
pump_relay_active
```

`device_unix_time_ms` means the device-provided Unix time, in milliseconds, at the moment the telemetry sample was created.

`mains_present` means the device observes that mains power is present at the monitored point.

`pump_relay_active` means the device observes that the monitored pump relay/contact signal is active.

The backend must treat payload values as reported observations from the device.

---

#### Backend Latest-State Store

The backend validates accepted telemetry and stores the latest known state for each device.

The backend stores or derives:

```text
latest mains_present value
latest pump_relay_active value
device_unix_time_ms
received_at_unix_time_ms
last_seen_unix_time_ms
validation result/status
offline status
```

The backend is the source of record for accepted telemetry and derived monitoring status.

The backend must not invent physical state that was not reported by the device.

---

#### User Monitoring View

The user can view the current monitoring state of the device.

The MVP view shows:

```text
device status: online / offline
last update time
mains presence
pump relay active state
```

`online`, and `offline`, are backend-derived monitoring statuses based on accepted telemetry and last-seen time.

They are not physical signals directly published by the device.

---

### 5.3 MVP Scope: Excluded

The following are intentionally excluded from the MVP:

* user authentication
* session security
* authorization checks
* Role-Based Access Control (RBAC)
* user-device assignment
* remote control
* Start/Stop commands
* command queues
* command retries
* command acknowledgements
* edge automation
* alerting
* advanced analytics
* historical data views
* pressure telemetry
* detailed electrical telemetry such as voltage/current

These are important future capabilities, but they are not part of the first secure monitoring proof.

---

### 5.4 MVP Device Responsibilities

For MVP, the device is responsible for:

* holding a provisioned device identity
* connecting to MQTT using mTLS
* publishing telemetry under its authenticated device identity
* observing mains presence
* observing pump relay/contact active state
* providing `device_unix_time_ms`
* publishing telemetry on first observation
* publishing telemetry when observed values change
* publishing heartbeat telemetry when values do not change

The device is not responsible for:

* authenticating users
* authorizing users
* assigning users to devices
* storing long-term telemetry history
* exposing a UI
* receiving remote control commands
* starting or stopping the pump remotely
* deciding backend offline status

---

### 5.5 MVP Backend Responsibilities

For MVP, the backend is responsible for:

* receiving telemetry from the broker/backend ingestion path
* validating device identity from the authenticated transport/topic context
* validating that telemetry belongs to the expected device
* validating telemetry message shape
* storing accepted telemetry
* rejecting or marking invalid telemetry
* storing latest known device state
* recording backend receive time
* deriving last-seen time
* deriving online/offline status
* exposing latest state to the monitoring view

The backend must distinguish between:

```text
device-reported field observations
device-provided wall-clock time
backend receive metadata
backend-derived monitoring status
```

---

### 5.6 MVP Device Telemetry Contract

MVP telemetry contains metadata and payload.

The device identity is not included in the telemetry payload. It is derived from the authenticated MQTT/mTLS connection, broker authorization, and topic context.

Example shape:

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

#### Metadata

`device_unix_time_ms`

Integer.

The device-provided Unix time, in milliseconds.

This represents the device’s wall-clock time at the moment the telemetry sample was created.

This is not the backend receive time and must not be used as the sole basis for offline status.

#### Payload

`mains_present`

Boolean.

Represents observed mains presence at the monitored point.

`pump_relay_active`

Boolean.

Represents observed pump relay/contact active state.

---

### 5.7 MVP Telemetry Publication Rules

The device should publish telemetry when:

* the first valid observation is available after boot/connect
* `mains_present` changes
* `pump_relay_active` changes
* the heartbeat interval elapses, even if values have not changed

The heartbeat allows the backend to distinguish a quiet but connected device from an offline device.

---

### 5.8 MVP Validation Rules

The backend should validate at least:

* the device identity is known
* the device identity from the authenticated transport context is valid
* the MQTT topic/device context is consistent with the authenticated device identity
* the payload has the expected schema
* `metadata.device_unix_time_ms` is an integer
* `payload.mains_present` is boolean
* `payload.pump_relay_active` is boolean
* invalid messages are rejected or marked invalid

The exact validation implementation may evolve, but the MVP must make invalid telemetry distinguishable from accepted telemetry.

---

### 5.9 MVP Offline Rules

The device does not decide whether it is offline.

The device publishes heartbeat telemetry.

The backend derives offline status from accepted telemetry arrival time.

The backend must not use `device_unix_time_ms` as the sole basis for freshness, or offline status.

If no accepted telemetry arrives within the configured timeout, the backend should show the device as offline.

An offline device state must not be displayed as a fresh confirmed physical state.

---

### 5.10 MVP Non-Goal Statement

The MVP is not a remote pump-control system.

The MVP is not a complete user security system.

The MVP is not an analytics platform.

The MVP is the smallest secure monitoring system that proves:

```text
provisioned device
→ authenticated telemetry transport
→ backend validation
→ latest-state storage
→ HTTPS monitoring view
```
