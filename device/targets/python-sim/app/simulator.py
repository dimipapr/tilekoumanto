import random
import time

from app.publisher import TelemetryPublisher
from ffi.core import (
    Telemetry,
    TK_MAINS_POWER_NOT_PRESENT,
    TK_MAINS_POWER_PRESENT,
    TK_PUMP_RELAY_ACTIVE,
    TK_PUMP_RELAY_INACTIVE,
)


class PythonSim:
    def __init__(self) -> None:
        self.mains_power = TK_MAINS_POWER_NOT_PRESENT
        self.pump_relay = TK_PUMP_RELAY_INACTIVE
        self.publisher = TelemetryPublisher.from_env()

    def unix_time_ms(self) -> int:
        return int(time.time() * 1000)

    def update_environment(self) -> None:
        self.mains_power = random.choice(
            [
                TK_MAINS_POWER_PRESENT,
                TK_MAINS_POWER_NOT_PRESENT,
            ]
        )

        self.pump_relay = random.choice(
            [
                TK_PUMP_RELAY_ACTIVE,
                TK_PUMP_RELAY_INACTIVE,
            ]
        )

    def read_telemetry(self, out: Telemetry) -> int:
        self.update_environment()

        out.contents.mains_power = self.mains_power
        out.contents.pump_relay = self.pump_relay
        out.contents.unix_time_ms = self.unix_time_ms()

        return 1

    def publish_telemetry(self, telemetry: Telemetry) -> int:
        return self.publisher.publish(telemetry)

    def close(self) -> None:
        self.publisher.close()