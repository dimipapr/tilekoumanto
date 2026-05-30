import json
from datetime import datetime, timezone
import paho.mqtt.client as mqtt
from django.core.management.base import BaseCommand
from devices.models import Device, DeviceMessageRaw
from django.utils import datetime, timezone

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
                topic_parts = msg.topic.split("/")

                if len(topic_parts) != 4:
                    self.stdout.write(self.style.WARNING(f"⚠️ Telemetry dropped. Invalid topic: {msg.topic}"))
                    return

                if topic_parts[0] != "devices" or topic_parts[2] != "pump" or topic_parts[3] != "telemetry":
                    self.stdout.write(self.style.WARNING(f"⚠️ Telemetry dropped. Unexpected topic: {msg.topic}"))
                    return

                device_uuid = topic_parts[1]

                raw_payload = json.loads(msg.payload.decode())

                metadata = raw_payload.get("meta")
                payload = raw_payload.get("payload")

                if not isinstance(metadata, dict):
                    self.stdout.write(self.style.WARNING("⚠️ Telemetry dropped. Missing or invalid meta object."))
                    return

                if not isinstance(payload, dict):
                    self.stdout.write(self.style.WARNING("⚠️ Telemetry dropped. Missing or invalid payload object."))
                    return

                unix_time_ms = metadata.get("unix_time_ms")

                if not isinstance(unix_time_ms, int):
                    self.stdout.write(self.style.WARNING("⚠️ Telemetry dropped. Missing or invalid meta.unix_time_ms."))
                    return

                if not isinstance(payload.get("mains_power_present"), bool):
                    self.stdout.write(self.style.WARNING("⚠️ Telemetry dropped. Missing or invalid payload.mains_power_present."))
                    return

                if not isinstance(payload.get("pump_relay_active"), bool):
                    self.stdout.write(self.style.WARNING("⚠️ Telemetry dropped. Missing or invalid payload.pump_relay_active."))
                    return

                device_timestamp = datetime.fromtimestamp(unix_time_ms / 1000, tz=timezone.utc)

                try:
                    device = Device.objects.get(uuid=device_uuid)
                except Device.DoesNotExist:
                    self.stdout.write(self.style.WARNING(f"⚠️ Telemetry dropped. Unknown device: {device_uuid}"))
                    return

                DeviceMessageRaw.objects.create(
                    device=device,
                    topic=msg.topic,
                    device_timestamp=device_timestamp,
                    payload=raw_payload
                )

                device.last_seen = timezone.now()
                device.save(update_fields=["last_seen"])

                self.stdout.write(self.style.SUCCESS(f"📥 Saved raw message for {device_uuid}"))

            except json.JSONDecodeError:
                self.stdout.write(self.style.WARNING("⚠️ Telemetry dropped. Payload is not valid JSON."))
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