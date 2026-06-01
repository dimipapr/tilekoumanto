import threading
from pathlib import Path

from dotenv import load_dotenv

from app.simulator import PythonSim
from ffi.core import (
    Core,
    LOG_CB,
    PUBLISH_TELEMETRY_CB,
    READ_TELEMETRY_CB,
    SHOULD_STOP_CB,
    UNIX_TIME_MS_CB,
    Platform,
)


def main() -> int:
    target_root = Path(__file__).resolve().parents[1]
    load_dotenv(target_root / ".env")

    core = Core()
    sim = PythonSim()
    stop_requested = threading.Event()

    def log(message: bytes) -> None:
        print(f"[python] {message.decode()}")

    def should_stop() -> int:
        return 1 if stop_requested.is_set() else 0

    log_cb = LOG_CB(log)
    unix_time_ms_cb = UNIX_TIME_MS_CB(sim.unix_time_ms)
    read_telemetry_cb = READ_TELEMETRY_CB(sim.read_telemetry)
    publish_telemetry_cb = PUBLISH_TELEMETRY_CB(sim.publish_telemetry)
    should_stop_cb = SHOULD_STOP_CB(should_stop)

    platform = Platform(
        log=log_cb,
        unix_time_ms=unix_time_ms_cb,
        read_telemetry=read_telemetry_cb,
        publish_telemetry=publish_telemetry_cb,
        should_stop=should_stop_cb,
    )

    print(f"tilekoumanto core version: {core.version()}")
    print("[python] starting simulator")

    def run_core() -> None:
        result = core.run(platform)
        print(f"[python] core returned: {result}")

    core_thread = threading.Thread(
        target=run_core,
        name="core",
        daemon=False,
    )

    core_thread.start()

    try:
        while core_thread.is_alive():
            core_thread.join(timeout=0.2)
    except KeyboardInterrupt:
        print("\n[python] stop requested")
        stop_requested.set()
        core_thread.join()
        print("[python] simulator stopped")
    finally:
        sim.close()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())