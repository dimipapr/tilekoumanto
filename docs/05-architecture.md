# Architecture

## 1. System purpose

Tilekoumanto monitors an agricultural irrigation pump installation remotely.

For the MVP, the system only needs to expose two field states:

- mains power state
- pump relay state

The purpose of the architecture is to move these states from the field installation to a backend system where they can be stored and viewed through an API.

## 2. MVP architecture

The MVP architecture consists of:

- an STM32-based field device
- external relay contacts for electrical state detection
- MQTT communication over LTE
- Mosquitto as the MQTT broker
- Django as the backend/API
- PostgreSQL as the database
- Caddy as the HTTP entrypoint/reverse proxy
- Docker Compose for local deployment

For the MVP, the farmer-facing interface is the API. No separate web dashboard or mobile application is included yet.

## 3. Field device

The field device is based on STM32.

Its role is to read electrical state signals from the pump installation and publish them to the backend through MQTT over LTE.

The device does not directly measure three-phase mains voltage. Mains presence is detected through the contact of an external voltage monitoring relay.

The device does not directly measure pump motor current or hydraulic behavior. Pump state is detected through a relay contact that represents whether the pump is commanded or considered running by the electrical control circuit.

## 4. Field inputs

### 4.1 Mains power state

Mains power state is detected through an external voltage monitoring relay contact.

The STM32 reads this contact as a discrete input and treats it as the field signal for whether mains power is available at the installation.

### 4.2 Pump relay state

Pump state is detected through a relay contact from the pump control circuit.

The STM32 reads this contact as a discrete input and treats it as the field signal for whether the pump relay is active.

## 5. Communication

The field device communicates with the backend using MQTT over LTE.

The device publishes field state updates to the MQTT broker. The backend consumes these messages and stores the reported state.

For the MVP, communication is one-way from the field device to the backend. Remote control is not included.

## 6. Backend

The backend stack consists of:

- Mosquitto for MQTT message handling
- Django for application logic and API
- PostgreSQL for persistent storage
- Caddy as the HTTP entrypoint and reverse proxy

Django is responsible for exposing the current system state through an API and storing received field state information in PostgreSQL.

## 7. Data storage

PostgreSQL stores the reported field state.

At minimum, the MVP needs to store:

- device identity
- timestamp of reported state
- mains power state
- pump relay state

The backend should be able to expose the latest known state for a device through the API.

## 8. Farmer-facing interface

For the MVP, the farmer-facing interface is the API.

The API should make the latest known field state available, including:

- whether mains power is available
- whether the pump relay is active
- when the state was last updated

A separate frontend can be added later, but is not part of the MVP architecture.

## 9. Deployment

The current deployment target is a local machine using Docker Compose.

The stack is intended to be transferable to a VPS without major architectural changes.

The local deployment includes:

- Caddy
- Django
- PostgreSQL
- Mosquitto

## 10. MVP data flow

```text
Field installation
→ relay contacts
→ STM32 field device
→ MQTT over LTE
→ Mosquitto
→ Django backend
→ PostgreSQL
→ API