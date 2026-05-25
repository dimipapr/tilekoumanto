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
