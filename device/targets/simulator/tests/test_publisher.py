from app.publisher import _mains_power_to_string, _pump_relay_to_string
from ffi.core import (
    TK_MAINS_POWER_FAULT,
    TK_MAINS_POWER_NOT_PRESENT,
    TK_MAINS_POWER_PRESENT,
    TK_PUMP_RELAY_ACTIVE,
    TK_PUMP_RELAY_FAULT,
    TK_PUMP_RELAY_INACTIVE,
)


def test_mains_power_mapping_smoke():
    assert _mains_power_to_string(TK_MAINS_POWER_PRESENT) == "present"
    assert _mains_power_to_string(TK_MAINS_POWER_NOT_PRESENT) == "not_present"
    assert _mains_power_to_string(TK_MAINS_POWER_FAULT) == "fault"


def test_pump_relay_mapping_smoke():
    assert _pump_relay_to_string(TK_PUMP_RELAY_ACTIVE) == "active"
    assert _pump_relay_to_string(TK_PUMP_RELAY_INACTIVE) == "inactive"
    assert _pump_relay_to_string(TK_PUMP_RELAY_FAULT) == "fault"