import os
import sys
import ssl
import json
import time
import random
import paho.mqtt.client as mqtt

# --- Configuration ---
BROKER = "mqtt-dev.tilekoumanto.gr"
PORT = 8883
TOPIC_TEMPLATE = "devices/{uuid}/pump/telemetry"

# --- Dynamic Path Resolution ---
CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(CURRENT_DIR)
CERTS_DIR = os.path.join(REPO_ROOT, "certs")

def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("✅ Successfully connected to Mosquitto (mTLS verified).")
    else:
        print(f"❌ Connection failed with reason code {reason_code}")

def on_publish(client, userdata, mid, reason_code, properties):
    print(f"📡 Message {mid} published successfully.")

def main(device_uuid):
    ca_crt = os.path.join(CERTS_DIR, "devices", device_uuid, "ca.crt")
    device_crt = os.path.join(CERTS_DIR, "devices", device_uuid, f"{device_uuid}.crt")
    device_key = os.path.join(CERTS_DIR, "devices", device_uuid, f"{device_uuid}.key")

    if not all(os.path.exists(p) for p in [ca_crt, device_crt, device_key]):
        print(f"❌ Error: Cannot find certificates for UUID {device_uuid}.")
        sys.exit(1)

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=f"sim-{device_uuid}")
    client.on_connect = on_connect
    client.on_publish = on_publish

    print("Configuring TLS context...")
    client.tls_set(
        ca_certs=ca_crt,
        certfile=device_crt,
        keyfile=device_key,
        cert_reqs=ssl.CERT_REQUIRED,
        tls_version=ssl.PROTOCOL_TLSv1_2
    )
    
    client.tls_insecure_set(False)

    print(f"Connecting to {BROKER}:{PORT} as {device_uuid}...")
    try:
        client.connect(BROKER, PORT, keepalive=60)
    except Exception as e:
        print(f"❌ Connection error: {e}")
        sys.exit(1)
    
    client.loop_start()
    topic = TOPIC_TEMPLATE.format(uuid=device_uuid)

    try:
        while True:
            payload = {
                "metadata": {
                    "device_uuid": device_uuid,
                    "timestamp": int(time.time()),
                },
                "payload": {
                    "mains_present": True,
                    "relay_active": True
                }
            }
            
            client.publish(topic, json.dumps(payload), qos=1)
            time.sleep(10)
            
    except KeyboardInterrupt:
        print("\n🛑 Stopping simulator...")
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    target_uuid = None
    
    # If a UUID is passed manually, use it
    if len(sys.argv) == 2:
        target_uuid = sys.argv[1]
    else:
        # Otherwise, grab a random one from the manifest
        manifest_path = os.path.join(CERTS_DIR, "devices", "manifest.json")
        try:
            with open(manifest_path, "r") as f:
                manifest = json.load(f)
                devices = manifest.get("devices", [])
                
                if not devices:
                    print("❌ Error: manifest.json is empty. No devices to simulate.")
                    sys.exit(1)
                    
                target_uuid = random.choice(devices)
                print(f"🎲 No UUID provided. Randomly selected: {target_uuid}\n")
        except FileNotFoundError:
            print("❌ Error: Cannot find manifest.json.")
            print("Run 'python operator/project.py certs <number>' first, or provide a UUID manually.")
            sys.exit(1)
            
    main(target_uuid)