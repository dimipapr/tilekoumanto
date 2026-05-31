import json
import random
import ssl
import time
from pathlib import Path

import paho.mqtt.client as mqtt


HOST = "mqtt-dev.tilekoumanto.gr"
PORT = 8883
CERTS_ROOT = Path("certs")
DEVICES_DIR = CERTS_ROOT / "devices"
MANIFEST_PATH = DEVICES_DIR / "manifest.json"
PUBLISH_INTERVAL_SECONDS = 10


def load_random_device_uuid() -> str:
    with MANIFEST_PATH.open("r", encoding="utf-8") as f:
        manifest = json.load(f)

    if isinstance(manifest, list):
        device_entries = manifest
    elif isinstance(manifest, dict) and "devices" in manifest:
        device_entries = manifest["devices"]
    else:
        raise ValueError("Unsupported device manifest shape.")

    if not device_entries:
        raise ValueError("Device manifest does not contain any devices.")

    selected = random.choice(device_entries)

    if isinstance(selected, str):
        return selected

    if isinstance(selected, dict) and "uuid" in selected:
        return selected["uuid"]

    raise ValueError("Unsupported device entry shape in manifest.")


def build_payload() -> str:
    return json.dumps(
        {
            "meta": {
                "unix_time_ms": int(time.time() * 1000),
            },
            "payload": {
                "mains_power_present": True,
                "pump_relay_active": random.choice([True,False]),
            },
        },
        separators=(",", ":"),
    )


def main() -> None:
    device_uuid = load_random_device_uuid()

    device_cert_dir = DEVICES_DIR / device_uuid
    ca_cert = device_cert_dir / "ca.crt"
    client_cert = device_cert_dir / f"{device_uuid}.crt"
    client_key = device_cert_dir / f"{device_uuid}.key"

    topic = f"devices/{device_uuid}/pump/telemetry"

    client = mqtt.Client(client_id=f"dev-sender-{device_uuid}")

    client.tls_set(
        ca_certs=str(ca_cert),
        certfile=str(client_cert),
        keyfile=str(client_key),
        cert_reqs=ssl.CERT_REQUIRED,
        tls_version=ssl.PROTOCOL_TLS_CLIENT,
    )

    client.connect(HOST, PORT, keepalive=60)
    client.loop_start()

    print(f"Publishing telemetry as device {device_uuid}")
    print(f"Topic: {topic}")
    print(f"Interval: {PUBLISH_INTERVAL_SECONDS}s")

    try:
        while True:
            payload = build_payload()

            result = client.publish(topic, payload, qos=1)
            result.wait_for_publish()

            print(f"Published: {payload}")

            time.sleep(PUBLISH_INTERVAL_SECONDS)

    except KeyboardInterrupt:
        print("Stopping sender...")

    finally:
        client.loop_stop()
        client.disconnect()


if __name__ == "__main__":
    main()