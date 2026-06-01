# backend/django/devices/management/commands/mqtt_catcher.py

import json
import signal
import threading
from datetime import datetime, timezone as dt_timezone

import paho.mqtt.client as mqtt
from django.core.management.base import BaseCommand
from django.utils import timezone
from pydantic import ValidationError

from devices.contracts.mqtt import (
    MainsPowerState,
    MqttMessageMeta,
    PumpRelayState,
    PumpTelemetryMessage,
)
from devices.models import Device, DeviceMessageRaw, PumpStateSample


class Command(BaseCommand):
    help = "Runs the MQTT Catcher to ingest raw telemetry data from Mosquitto"

    def handle(self, *args, **options):
        broker = "mosquitto"
        port = 1883

        client = mqtt.Client()

        def on_connect(client, userdata, flags, rc):
            if rc == 0:
                self.stdout.write(
                    self.style.SUCCESS(
                        "✅ Worker connected to internal Mosquitto (Cleartext)."
                    )
                )
                client.subscribe("devices/+/pump/telemetry")
                self.stdout.write("🎧 Subscribed to: devices/+/pump/telemetry")
            else:
                self.stdout.write(
                    self.style.ERROR(f"❌ Connection failed with code {rc}")
                )

        def on_message(client, userdata, msg):
            try:
                topic_parts = msg.topic.split("/")

                if len(topic_parts) != 4:
                    self.stdout.write(
                        self.style.WARNING(
                            f"⚠️ Telemetry dropped. Invalid topic: {msg.topic}"
                        )
                    )
                    return

                device_uuid = topic_parts[1]

                try:
                    device = Device.objects.get(uuid=device_uuid)
                except Device.DoesNotExist:
                    self.stdout.write(
                        self.style.WARNING(
                            f"⚠️ Telemetry dropped. Unknown device: {device_uuid}"
                        )
                    )
                    return

                try:
                    raw_payload = json.loads(msg.payload.decode())
                except json.JSONDecodeError:
                    self.stdout.write(
                        self.style.WARNING(
                            "⚠️ Telemetry dropped. Payload is not valid JSON."
                        )
                    )
                    return

                if not isinstance(raw_payload, dict):
                    self.stdout.write(
                        self.style.WARNING(
                            "⚠️ Telemetry dropped. JSON payload must be an object."
                        )
                    )
                    return

                unix_time_ms = None

                try:
                    meta = MqttMessageMeta.model_validate(raw_payload.get("meta"))
                    unix_time_ms = meta.unix_time_ms
                except ValidationError:
                    pass

                raw_message = DeviceMessageRaw.objects.create(
                    device=device,
                    topic=msg.topic,
                    device_unix_time_ms=unix_time_ms,
                    payload=raw_payload,
                )

                try:
                    message = PumpTelemetryMessage.model_validate(raw_payload)
                except ValidationError as e:
                    self.stdout.write(
                        self.style.WARNING(
                            f"⚠️ Raw message saved, but pump telemetry rejected: {e}"
                        )
                    )
                    return

                device_timestamp = datetime.fromtimestamp(
                    message.meta.unix_time_ms / 1000,
                    tz=dt_timezone.utc,
                )

                PumpStateSample.objects.create(
                    device=device,
                    raw_message=raw_message,
                    device_timestamp=device_timestamp,
                    mains_power_present=(
                        message.payload.readings.mains_power
                        == MainsPowerState.PRESENT
                    ),
                    pump_relay_active=(
                        message.payload.readings.pump_relay
                        == PumpRelayState.ACTIVE
                    ),
                )

                device.last_seen = timezone.now()
                device.save(update_fields=["last_seen"])

                self.stdout.write(
                    self.style.SUCCESS(f"📥 Saved pump telemetry for {device_uuid}")
                )

            except Exception as e:
                self.stdout.write(
                    self.style.ERROR(f"❌ Error processing message: {e}")
                )

        client.on_connect = on_connect
        client.on_message = on_message

        stop_event = threading.Event()

        def request_stop(signum, frame):
            self.stdout.write("Stopping MQTT catcher...")
            stop_event.set()
            client.disconnect()

        signal.signal(signal.SIGTERM, request_stop)
        signal.signal(signal.SIGINT, request_stop)

        self.stdout.write("Connecting Worker to Internal Broker...")

        try:
            client.connect(broker, port, 60)
            client.loop_start()

            while not stop_event.is_set():
                stop_event.wait(1)

        except Exception as e:
            self.stdout.write(self.style.ERROR(f"❌ Connection error: {e}"))

        finally:
            client.loop_stop()
            self.stdout.write("MQTT catcher stopped.")