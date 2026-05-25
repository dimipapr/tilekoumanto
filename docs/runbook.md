
# Certificate Management

This runbook details how to generate and manage the Public Key Infrastructure (PKI) required for MQTT over mTLS using the included Operator CLI.

## Overview

The `operator/project.py` script automates the creation of a local Certificate Authority (CA), the Mosquitto server certificates, and batched device certificates based on UUIDs.

It acts idempotently based on a **target count**: it will count the existing devices in the manifest and only generate the difference needed to reach your requested target.

## Prerequisites

* **Python 3.x**
* **OpenSSL** installed and available in your system's PATH.

## Usage

The certificate generator requires a mandatory target count argument. Run the script from anywhere; it will automatically resolve the repository root and output to the `certs/` directory.

### Basic Commands

**Generate certificates for a target of 50 devices:**

```bash
python operator/project.py certs 50

```

*If starting from zero, this generates the CA, Server cert, and 50 device certs. If you already have 50 devices, it does nothing.*

**Expand your device pool to 75:**

```bash
python operator/project.py certs 75

```

*If you currently have 50 devices, this will only generate 25 new certificates.*

**View CLI Help:**

```bash
python operator/project.py certs -h

```

---

## Output Directory Structure

The script dynamically creates a `certs/` directory at the root of the repository.

```text
certs/
├── ca.crt                  # Root CA Certificate (Distribute to Mosquitto and Devices)
├── ca.key                  # Root CA Private Key (KEEP SECURE/OFFLINE)
├── server.crt              # Mosquitto Server Certificate (Signed by CA)
├── server.key              # Mosquitto Server Private Key
└── devices/
    ├── manifest.json       # Array of all generated device UUID strings
    ├── <device-uuid-1>/
    │   ├── <device-uuid-1>.crt  # Device Certificate (CN = UUID)
    │   └── <device-uuid-1>.key  # Device Private Key
    └── <device-uuid-N>/
        ├── <device-uuid-N>.crt
        └── <device-uuid-N>.key

```

---

## Security & Deployment Notes

* **Never commit the `certs/` directory to version control.** Ensure `certs/` is added to your `.gitignore`.
* **Broker Configuration:** Mosquitto requires `ca.crt`, `server.crt`, and `server.key`. It uses `ca.crt` to verify incoming device connections.
* **Device Configuration:** Each physical device or simulator requires the shared `ca.crt` alongside its specific `<uuid>.crt` and `<uuid>.key`.
* **Identity Enforcement:** Mosquitto must be configured with `use_identity_as_username true` so the backend MQTT worker can extract the UUID from the connection and authorize topic publication.