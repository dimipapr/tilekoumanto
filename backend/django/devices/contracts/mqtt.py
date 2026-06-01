#backend/django/devices/contracts/mqtt.py

from pydantic import BaseModel, ConfigDict, StrictBool, StrictInt

class MqttMessageMeta(BaseModel):
    model_config = ConfigDict(extra="forbid")

    unix_time_ms: StrictInt

class PumpTelemetryPayload(BaseModel):
    model_config = ConfigDict(extra="forbid")

    mains_power_present: StrictBool
    pump_relay_active: StrictBool

class PumpTelemetryMessage(BaseModel):
    model_config = ConfigDict(extra="forbid")

    meta: MqttMessageMeta
    payload: PumpTelemetryPayload