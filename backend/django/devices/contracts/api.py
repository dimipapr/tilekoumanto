# backend/django/devices/contracts/api.py

from datetime import datetime
from uuid import UUID

from pydantic import BaseModel


class LatestDeviceStateResponse(BaseModel):
    device_uuid: UUID
    mains_power_present: bool
    pump_relay_active: bool
    device_reported_at: datetime
    backend_received_at: datetime