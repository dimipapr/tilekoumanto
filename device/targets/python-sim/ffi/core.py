import ctypes
from pathlib import Path


DEFAULT_CORE_LIBRARY_PATH = (
    Path(__file__).resolve().parents[1]
    / "build"
    / "debug"
    / "core"
    / "libtilekoumanto_core.so"
)


LOG_CB = ctypes.CFUNCTYPE(None, ctypes.c_char_p)
UNIX_TIME_MS_CB = ctypes.CFUNCTYPE(ctypes.c_uint64)


class Platform(ctypes.Structure):
    _fields_ = [
        ("log", LOG_CB),
        ("unix_time_ms", UNIX_TIME_MS_CB),
    ]


class Core:
    def __init__(self, library_path: Path | None = None) -> None:
        path = library_path or DEFAULT_CORE_LIBRARY_PATH

        if not path.exists():
            raise FileNotFoundError(
                f"Core shared library not found: {path}\n"
                "Build it first with:\n"
                "  cd device/targets/python-sim\n"
                "  cmake --preset debug\n"
                "  cmake --build --preset debug"
            )

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