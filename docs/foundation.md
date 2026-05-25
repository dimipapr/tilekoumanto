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

A provisioned, authenticated device can securely send field readings to the backend; the backend validates, stores, and exposes those readings; and a user can securely view the current device state over HTTPS.

### 5.2 MVP Scope (Included)

* **Device Provisioning:** Device identity.
* **Secure Device Communication:** MQTT over mTLS, device certificate identity validation, broker-side authentication, backend message validation.
* **Secure User Communication:** HTTPS via Caddy.
* **Telemetry Ingestion:** Main relay state, mains presence, device timestamp.
* **Backend Source of Truth:** Latest known device state, last seen time, validation results.
* **User Monitoring View:** Device status (online/offline/stale), last update time, mains presence, main relay active state.

### 5.3 MVP Scope (Excluded)

* User authentication, session security, and authorization checks.
* Role-Based Access Control (RBAC) and user-device assignment.
* Remote control (Start/Stop commands, queues, retries).
* Edge automation and alerting.
* Advanced analytics and historical data views.
* Pressure and detailed electrical telemetry (voltage/current).