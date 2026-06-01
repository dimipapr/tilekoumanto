import ctypes
from pathlib import Path


PYTHON_TARGET_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_CORE_LIBRARY_PATH = (
    PYTHON_TARGET_ROOT
    / "build"
    / "debug"
    / "core"
    / "libtilekoumanto_core.so"
)


TK_MAINS_POWER_PRESENT = 0
TK_MAINS_POWER_NOT_PRESENT = 1
TK_MAINS_POWER_FAULT = 2

TK_PUMP_RELAY_ACTIVE = 0
TK_PUMP_RELAY_INACTIVE = 1
TK_PUMP_RELAY_FAULT = 2


class Telemetry(ctypes.Structure):
    _fields_ = [
        ("mains_power", ctypes.c_int),
        ("pump_relay", ctypes.c_int),
        ("unix_time_ms", ctypes.c_uint64),
    ]


LOG_CB = ctypes.CFUNCTYPE(None, ctypes.c_char_p)
UNIX_TIME_MS_CB = ctypes.CFUNCTYPE(ctypes.c_uint64)
READ_TELEMETRY_CB = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(Telemetry))
PUBLISH_TELEMETRY_CB = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(Telemetry))
SHOULD_STOP_CB = ctypes.CFUNCTYPE(ctypes.c_int)

class Platform(ctypes.Structure):
    _fields_ = [
        ("log", LOG_CB),
        ("unix_time_ms", UNIX_TIME_MS_CB),
        ("read_telemetry", READ_TELEMETRY_CB),
        ("publish_telemetry", PUBLISH_TELEMETRY_CB),
        ("should_stop", SHOULD_STOP_CB),
    ]


class Core:
    def __init__(self, library_path: Path | None = None) -> None:
        path = library_path or DEFAULT_CORE_LIBRARY_PATH

        if not path.exists():
            raise FileNotFoundError(
                f"Core shared library not found: {path}\n")

        self._lib = ctypes.CDLL(str(path))

        self._lib.tk_core_version.argtypes = []
        self._lib.tk_core_version.restype = ctypes.c_int

        self._lib.tk_core_run.argtypes = [
            ctypes.POINTER(Platform),
        ]
        self._lib.tk_core_run.restype = ctypes.c_int

    def version(self) -> int:
        return int(self._lib.tk_core_version())

    def run(self, platform: Platform) -> bool:
        return bool(self._lib.tk_core_run(ctypes.byref(platform)))