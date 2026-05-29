# Tilekoumanto

> **Disclaimer:** This project is in a very early stage of development. The system architecture and features are highly experimental and far from the final target state.

Tilekoumanto is a full-stack IoT platform designed for remote telemetry ingestion and edge device control. It features a Django/PostgreSQL backend, an Eclipse Mosquitto MQTT broker, and a FreeRTOS-based C core for embedded edge devices (STM32F446RE) and Linux simulation.

## System Architecture

* **Ingress (Caddy):** Handles external HTTPS traffic with automated Cloudflare DNS-01 challenges and serves Django static files.
* **Message Broker (Mosquitto):** Dual-listener configuration. Exposes a strict mTLS listener on port 8883 for external edge devices and a cleartext listener on port 1883 strictly for internal backend services.
* **Web Server (Django):** Provides the API and administrative interfaces.
* **Telemetry Ingestion (Django Management Command):** A dedicated worker (`mqtt_catcher`) connects to the internal Mosquitto broker to asynchronously ingest device data into PostgreSQL.
* **Edge Core (C/FreeRTOS):** Cross-platform core logic utilizing strict static memory allocation. Compiles to an ARM Cortex-M4F ELF or a Linux shared object (`.so`) for Python-based simulation.

---

## 1. Backend Deployment

### Prerequisites

* Docker & Docker Compose
* A Cloudflare API Token (for DNS challenges)
* mTLS Certificates for devices

### Environment Variables

Create an `.env` file in the root of the project:

```env
# Networking & DNS
DNS_API_TOKEN=your_cloudflare_api_token
DJANGO_ALLOWED_HOSTS=dev.example.com

# Django Settings
DJANGO_SECRET_KEY=your_secure_secret_key
DJANGO_DEBUG=false

# PostgreSQL Database
POSTGRES_DB=tilekoumanto
POSTGRES_USER=tk_user
POSTGRES_PASSWORD=secure_db_password

```

### Certificate Provisioning

Before starting the containers, generate the necessary mTLS certificates and the device manifest using the provisioning script:

```bash
python3 operator/project.py certs
```

Ensure the script has successfully populated the following structure at the project root:

* `certs/mosquitto/ca.crt`
* `certs/mosquitto/server.crt`
* `certs/mosquitto/server.key`
* `certs/devices/manifest.json`
* `certs/devices/<UUID>/...`

### Starting the Services

```bash
cd backend
docker compose up --build -d

```

### Initializing the Database & Devices

Once the containers are running, apply migrations and sync your provisioned devices from the manifest:

```bash
docker compose exec django-web python manage.py migrate
docker compose exec django-web python manage.py load_device_manifest

```

---

## 2. Device Simulator

The simulator compiles the FreeRTOS C core into a Linux shared library and uses a Python wrapper to mock hardware interactions.

### Prerequisites

* CMake (3.16+)
* Python 3.13+
* `paho-mqtt` Python package

### Build & Run

1. **Compile the Core for linux:**
```bash
cd device
mkdir build && cd build
cmake ..
make

```


2. **Execute the Simulator:**
Provide the target MQTT broker and the device UUID provisioned in your manifest.
```bash
cd ..
MQTT_HOST=mqtt-dev.example.com SIM_UUID=your-device-uuid python3 simulator.py

```



---

## 3. Hardware Compilation (STM32F446RE)

To build the embedded binary for the NUCLEO-F446RE board:

### Prerequisites

* ARM GNU Toolchain (`arm-none-eabi-gcc`)

### Build Instructions

```bash
cd device
mkdir build_hw && cd build_hw
cmake -DTARGET_PLATFORM=EMBEDDED -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake ..
make

```

The output will be an executable `tk_core.elf` alongside memory utilization metrics for Flash and SRAM.