import json
import os
import ssl
from dataclasses import dataclass
from pathlib import Path

import paho.mqtt.client as mqtt

from ffi.core import (
    Telemetry,
    TK_MAINS_POWER_FAULT,
    TK_MAINS_POWER_NOT_PRESENT,
    TK_MAINS_POWER_PRESENT,
    TK_PUMP_RELAY_ACTIVE,
    TK_PUMP_RELAY_FAULT,
    TK_PUMP_RELAY_INACTIVE,
)


def _mains_power_to_string(value: int) -> str:
    if value == TK_MAINS_POWER_PRESENT:
        return "present"

    if value == TK_MAINS_POWER_NOT_PRESENT:
        return "not_present"

    if value == TK_MAINS_POWER_FAULT:
        return "fault"

    return "fault"


def _pump_relay_to_string(value: int) -> str:
    if value == TK_PUMP_RELAY_ACTIVE:
        return "active"

    if value == TK_PUMP_RELAY_INACTIVE:
        return "inactive"

    if value == TK_PUMP_RELAY_FAULT:
        return "fault"

    return "fault"


@dataclass(frozen=True)
class MqttConfig:
    host: str
    port: int
    device_uuid: str
    topic: str
    client_id: str
    ca_cert: Path
    client_cert: Path
    client_key: Path


class TelemetryPublisher:
    def __init__(self, config: MqttConfig) -> None:
        self._config = config
        self._client = mqtt.Client(client_id=config.client_id)

        self._client.tls_set(
            ca_certs=str(config.ca_cert),
            certfile=str(config.client_cert),
            keyfile=str(config.client_key),
            tls_version=ssl.PROTOCOL_TLS_CLIENT,
        )

        self._client.connect(config.host, config.port, keepalive=60)
        self._client.loop_start()

    @classmethod
    def from_env(cls) -> "TelemetryPublisher":
        device_uuid = os.environ["TK_DEVICE_UUID"]
        host = os.environ["TK_MQTT_HOST"]
        port = int(os.environ["TK_MQTT_PORT"])

        certs_root = Path(os.environ["TK_CERTS_ROOT"])
        device_certs_dir = certs_root / "devices" / device_uuid

        ca_cert = device_certs_dir / "ca.crt"
        client_cert = device_certs_dir / f"{device_uuid}.crt"
        client_key = device_certs_dir / f"{device_uuid}.key"

        required_paths = [
            ca_cert,
            client_cert,
            client_key,
        ]

        missing_paths = [path for path in required_paths if not path.exists()]

        if missing_paths:
            missing = "\n".join(str(path) for path in missing_paths)
            raise FileNotFoundError(
                "Missing device certificate file(s):\n"
                f"{missing}"
            )

        return cls(
            MqttConfig(
                host=host,
                port=port,
                device_uuid=device_uuid,
                topic=f"devices/{device_uuid}/pump/telemetry",
                client_id=device_uuid,
                ca_cert=ca_cert,
                client_cert=client_cert,
                client_key=client_key,
            )
        )

    def publish(self, telemetry: Telemetry) -> int:
        value = telemetry.contents

        payload = {
            "meta": {
                "unix_time_ms": int(value.unix_time_ms),
            },
            "payload": {
                "readings": {
                    "mains_power": _mains_power_to_string(value.mains_power),
                    "pump_relay": _pump_relay_to_string(value.pump_relay),
                },
                "faults": [],
            },
        }

        encoded_payload = json.dumps(
            payload,
            separators=(",", ":"),
        )

        result = self._client.publish(
            self._config.topic,
            encoded_payload,
            qos=1,
            retain=False,
        )

        result.wait_for_publish()

        if result.rc != mqtt.MQTT_ERR_SUCCESS:
            return 0

        return 1

    def close(self) -> None:
        self._client.loop_stop()
        self._client.disconnect()