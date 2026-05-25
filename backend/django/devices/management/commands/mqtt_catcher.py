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
        # The internal Docker DNS name for the broker
        BROKER = "mosquitto"
        PORT = 1883  # Connecting to the internal cleartext listener

        client = mqtt.Client()

        def on_connect(client, userdata, flags, rc):
            if rc == 0:
                self.stdout.write(self.style.SUCCESS("✅ Worker connected to internal Mosquitto (Cleartext)."))
                client.subscribe("devices/+/pump/telemetry")
                self.stdout.write("🎧 Subscribed to: devices/+/pump/telemetry")
            else:
                self.stdout.write(self.style.ERROR(f"❌ Connection failed with code {rc}"))

        def on_message(client, userdata, msg):
            try:
                raw_payload = json.loads(msg.payload.decode())
                metadata = raw_payload.get("metadata", {})
                
                device_uuid = metadata.get("device_uuid")
                timestamp_unix = metadata.get("timestamp")
                dt = datetime.fromtimestamp(timestamp_unix, tz=timezone.utc)

                try:
                    device = Device.objects.get(uuid=device_uuid)
                except Device.DoesNotExist:
                    self.stdout.write(self.style.WARNING(f"⚠️ Telemetry dropped. Unknown device: {device_uuid}"))
                    return 

                DeviceMessageRaw.objects.create(
                    device=device,
                    topic=msg.topic,
                    device_timestamp=dt,
                    payload=raw_payload
                )

                device.last_seen = dt
                device.save(update_fields=['last_seen'])

                self.stdout.write(self.style.SUCCESS(f"📥 Saved raw message for {device_uuid}"))

            except Exception as e:
                self.stdout.write(self.style.ERROR(f"❌ Error processing message: {e}"))

        client.on_connect = on_connect
        client.on_message = on_message

        self.stdout.write("Connecting Worker to Internal Broker...")
        
        try:
            # Notice we completely removed client.tls_set()
            client.connect(BROKER, PORT, 60)
            client.loop_forever()
        except Exception as e:
            self.stdout.write(self.style.ERROR(f"❌ Connection error: {e}"))