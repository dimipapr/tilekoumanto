#backend/django/devices/contracts/mqtt.py

from enum import StrEnum

from pydantic import BaseModel, Field, ConfigDict, StrictInt

class MainsPowerState(StrEnum):
    PRESENT = "present"
    NOT_PRESENT = "not_present"
    FAULT = "fault"

class PumpRelayState(StrEnum):
    ACTIVE = "active"
    INACTIVE = "inactive"
    FAULT = "fault"

class FaultTarget(StrEnum):
    MAINS_POWER = "mains_power"
    PUMP_RELAY = "pump_relay"

class FaultType(StrEnum):
    UNREADABLE = "unreadable"

class MqttMessageMeta(BaseModel):
    model_config = ConfigDict(extra="forbid")

    unix_time_ms: StrictInt
    seq: StrictInt = Field(ge=0, le=4294967295)

class PumpTelemetryReadings(BaseModel):
    model_config = ConfigDict(extra="forbid")

    mains_power: MainsPowerState
    pump_relay: PumpRelayState

class PumpTelemetryFault(BaseModel):
    model_config = ConfigDict(extra="forbid")

    target: FaultTarget
    type: FaultType

class PumpTelemetryPayload(BaseModel):
    model_config = ConfigDict(extra="forbid")

    readings: PumpTelemetryReadings
    faults: list[PumpTelemetryFault]


class PumpTelemetryMessage(BaseModel):
    model_config = ConfigDict(extra="forbid")

    meta: MqttMessageMeta
    payload: PumpTelemetryPayload