import os
import sys
import ssl
import json
from datetime import datetime, timezone
import paho.mqtt.client as mqtt
from django.core.management.base import BaseCommand
from devices.models import Device, DeviceMessageRaw

BROKER = "mqtt-dev.tilekoumanto.gr" 
PORT = 8883
TOPIC = "devices/+/pump/telemetry"

class Command(BaseCommand):
    help = 'Runs the MQTT Catcher to ingest raw telemetry data from Mosquitto'

    def handle(self, *args, **options):
        # 1. Borrow a certificate to get past the mTLS bouncer
        certs_dir = "/certs/devices"
        manifest_path = os.path.join(certs_dir, "manifest.json")
        
        try:
            with open(manifest_path, "r") as f:
                manifest = json.load(f)
                borrowed_uuid = manifest["devices"][0]
        except (FileNotFoundError, IndexError):
            self.stdout.write(self.style.ERROR("❌ Cannot find manifest.json or devices to borrow a certificate."))
            sys.exit(1)

        ca_crt = os.path.join(certs_dir, borrowed_uuid, "ca.crt")
        device_crt = os.path.join(certs_dir, borrowed_uuid, f"{borrowed_uuid}.crt")
        device_key = os.path.join(certs_dir, borrowed_uuid, f"{borrowed_uuid}.key")

        # 2. Define MQTT Callbacks
        def on_connect(client, userdata, flags, reason_code, properties):
            if reason_code == 0:
                self.stdout.write(self.style.SUCCESS("✅ Catcher connected to Mosquitto (mTLS verified)."))
                client.subscribe(TOPIC)
                self.stdout.write(self.style.SUCCESS(f"🎧 Subscribed to: {TOPIC}"))
            else:
                self.stdout.write(self.style.ERROR(f"❌ Connection failed with reason code {reason_code}"))

        def on_message(client, userdata, msg):
            try:
                # Load the full JSON message
                raw_payload = json.loads(msg.payload.decode())
                metadata = raw_payload.get("metadata", {})
                
                device_uuid = metadata.get("device_uuid")
                timestamp_unix = metadata.get("timestamp")
                
                # Convert Unix timestamp to an aware UTC datetime object
                dt = datetime.fromtimestamp(timestamp_unix, tz=timezone.utc)

                # STRICT ENFORCEMENT: Only accept telemetry from provisioned devices
                try:
                    device = Device.objects.get(uuid=device_uuid)
                except Device.DoesNotExist:
                    self.stdout.write(self.style.WARNING(f"⚠️ Warning: Telemetry dropped. Unknown device UUID: {device_uuid}"))
                    return  # Abort saving this message

                # Dump the entire message as it comes
                DeviceMessageRaw.objects.create(
                    device=device,
                    topic=msg.topic,
                    device_timestamp=dt,
                    payload=raw_payload
                )

                # Update the device's last_seen
                device.last_seen = dt
                device.save(update_fields=['last_seen'])

                self.stdout.write(self.style.SUCCESS(f"📥 Saved raw message for {device_uuid}"))

            except Exception as e:
                self.stdout.write(self.style.ERROR(f"❌ Error processing message: {e}"))

        # 3. Setup and Run the Client
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="django-catcher")
        client.on_connect = on_connect
        client.on_message = on_message

        client.tls_set(
            ca_certs=ca_crt,
            certfile=device_crt,
            keyfile=device_key,
            cert_reqs=ssl.CERT_REQUIRED,
            tls_version=ssl.PROTOCOL_TLSv1_2
        )
        
        self.stdout.write("Connecting Catcher to Broker...")
        try:
            client.connect(BROKER, PORT, keepalive=60)
            client.loop_forever()
        except KeyboardInterrupt:
            self.stdout.write(self.style.WARNING("\n🛑 Stopping catcher..."))
            client.disconnect()
        except Exception as e:
            self.stdout.write(self.style.ERROR(f"❌ Connection error: {e}"))
            sys.exit(1)