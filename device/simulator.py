import ctypes
import os
import sys
import time
import threading
import paho.mqtt.client as mqtt
from pathlib import Path

lib_path = os.path.join(os.path.dirname(__file__), 'build', f'libtk_core.so')

try:
    tk_core = ctypes.CDLL(lib_path)
except OSError as e:
    print(f"Failed to load library at {lib_path}.\n Error: {e}")
    sys.exit(1)



MQTT_HOST = os.environ["MQTT_HOST"]
MQTT_PORT = 8883
DEVICE_UUID = os.environ["SIM_UUID"]

PROJECT_ROOT = Path(__file__).resolve().parents[1]
print(PROJECT_ROOT)
lib_path = PROJECT_ROOT / 'device' / 'build' / 'libtk_core.so'
CERTS_FOLDER = PROJECT_ROOT / "certs" / "devices" / DEVICE_UUID

mqtt_client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
print(str(CERTS_FOLDER / f"{DEVICE_UUID}.crt"))

mqtt_client.tls_set(
    ca_certs=str(CERTS_FOLDER / "ca.crt"),
    certfile=str(CERTS_FOLDER / f"{DEVICE_UUID}.crt"),
    keyfile=str(CERTS_FOLDER / f"{DEVICE_UUID}.key"),
)
print(f"Connecting to MQTT broker at {MQTT_HOST}...")
mqtt_client.connect(MQTT_HOST, MQTT_PORT, 60)
mqtt_client.loop_start()



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
    state_ptr.contents.mains_present = True
    state_ptr.contents.relay_active = False

def mqtt_publish(topic, payload):
    t = topic.decode('utf-8')
    p = payload.decode('utf-8')
    print(f"[MQTT OUT] {t} - >  {p}")
    mqtt_client.publish(t,p)

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

#wire stop function
tk_core.tk_core_stop.argtypes = []
tk_core.tk_core_stop.restype = None

def run_rtos_engine():
    print("[Simulator] Booting FreeRTOS scheduler inside background thread ...")

    tk_core.tk_core_init(
        DEVICE_UUID.encode('utf-8'),
        cb_telemetry,
        cb_publish,
        cb_time,
        cb_lock,
        cb_unlock,
    )

#instantiate and start the thread managing the C engine
#use daemon=True to kill thread with Ctrl+C
rtos_thread = threading.Thread(target=run_rtos_engine, daemon=False)
rtos_thread.start()

print("Simulator initialized and running. Press Ctrl+C to exit.")

#Keep the main python thread awake so the proccess stays alive??
#Why is that?

try:
    while True:
        time.sleep(0.1)
except KeyboardInterrupt:
    print("\nExiting Simulator.")

# Initialize Core
tk_core.tk_core_init.restype = None
tk_core.tk_core_init.argtypes = [
    ctypes.c_char_p, GET_TELEMETRY_CB, MQTT_PUBLISH_CB, GET_UNIX_TIME_CB, LOCK_CB, UNLOCK_CB
]

tk_core.tk_core_init(
    DEVICE_UUID.encode('utf-8'),
    cb_telemetry,
    cb_publish,
    cb_time,
    cb_lock,
    cb_unlock,
)

print("Simulator initialized. Running tick loop...")

tk_core.tk_core_tick.restype = None
tk_core.tk_core_tick.argtypes = []

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("Exiting simulator.")
    # tk_core.tk_core_stop()

    mqtt_client.loop_stop()
    mqtt_client.disconnect()

    os._exit(0)
