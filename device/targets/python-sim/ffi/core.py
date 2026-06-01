import ctypes
from pathlib import Path


import ctypes
from pathlib import Path


def _default_core_library_path() -> Path:
    return (
        Path(__file__).resolve().parents[1]
        / "build"
        / "debug"
        / "core"
        / "libtilekoumanto_core.so"
    )


class Core:
    def __init__(self, library_path: Path | None = None) -> None:
        path = library_path or _default_core_library_path()
        self._lib = ctypes.CDLL(str(path))

        self._lib.tk_core_version.argtypes = []
        self._lib.tk_core_version.restype = ctypes.c_int

    def version(self) -> int:
        return int(self._lib.tk_core_version())