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


class Core:
    def __init__(self, library_path: Path | None = None) -> None:
        path = library_path or DEFAULT_CORE_LIBRARY_PATH

        if not path.exists():
            raise FileNotFoundError(
                f"Core shared library not found: {path}\n"
                "Build it first with:\n"
                "  cd device/targets/python\n"
                "  cmake --preset debug\n"
                "  cmake --build --preset debug"
            )

        self._lib = ctypes.CDLL(str(path))

        self._lib.tk_core_version.argtypes = []
        self._lib.tk_core_version.restype = ctypes.c_int

        self._lib.tk_should_publish_telemetry.argtypes = [
            ctypes.POINTER(Telemetry),
            ctypes.POINTER(Telemetry),
        ]
        self._lib.tk_should_publish_telemetry.restype = ctypes.c_int

    def version(self) -> int:
        return int(self._lib.tk_core_version())

    def should_publish_telemetry(
        self,
        last_published: Telemetry | None,
        current: Telemetry | None,
    ) -> bool:
        last_ptr = (
            ctypes.byref(last_published)
            if last_published is not None
            else None
        )
        current_ptr = (
            ctypes.byref(current)
            if current is not None
            else None
        )

        return bool(self._lib.tk_should_publish_telemetry(last_ptr, current_ptr))