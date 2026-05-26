import ctypes
import os
import sys
import time

lib_path = os.path.join(os.path.dirname(__file__), 'build', f'libtk_core.so')

try:
    tk_core = ctypes.CDLL(lib_path)
except OSError as e:
    print(f"Failed to load library at {lib_path}.\n Error: {e}")
    sys.exit(1)

#mirror the c struct
class TkTelemetry(ctypes.Structure):
    _fields_ = [
        ("mains_present", ctypes.c_bool),
        ("relay_active", ctypes.c_bool)
    ]

#define callback signatures
GET_TELEMETRY_CB  = ctypes.CFUNCTYPE(None, ctypes.POINTER(TkTelemetry))
MQTT_PUBLISH_CB = ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_char_p)
GET_UNIX_TIME_CB = ctypes.CFUNCTYPE(ctypes.c_uint64)
LOCK_CB = ctypes.CFUNCTYPE(None)
UNLOCK_CB = ctypes.CFUNCTYPE(None)

#implement python mock functions
def get_telemetry(state_ptr):
    state_ptr.contents.main_present = True
    state_ptr.contents.relay_actuve = False

def mqtt_publish(topic, payload):
    print(f"[MQTT OUT] {topic.decode('utf-8')}->{payload.decode('utf-8')}")

def get_unix_time():
    #return in miliseconds to match C logic
    return int(time.time() * 1000)

def lock():
    pass

def unlock():
    pass

# Keep references to callbacks so Python's garbage collector doesn't nuke them
cb_telemetry = GET_TELEMETRY_CB(get_telemetry)
cb_publish = MQTT_PUBLISH_CB(mqtt_publish)
cb_time = GET_UNIX_TIME_CB(get_unix_time)
cb_lock = LOCK_CB(lock)
cb_unlock = UNLOCK_CB(unlock)

# Initialize Core

tk_core.tk_core_init.argtypes = [
    ctypes.c_char_p, GET_TELEMETRY_CB, MQTT_PUBLISH_CB, GET_UNIX_TIME_CB, LOCK_CB, UNLOCK_CB
]

tk_core.tk_core_init(
    b"sim-device-1234",
    cb_telemetry,
    cb_publish,
    cb_time,
    cb_lock,
    cb_unlock,
)

print("Simulator initialized. Running tick loop...")

tk_core.tk_core_tick.sertype = None
tk_core.tk_core_tick.argtypes = []

try:
    while True:
        tk_core.tk_core_tick()
        time.sleep(0.01)
except KeyboardInterrupt:
    print("Exiting simulator.")