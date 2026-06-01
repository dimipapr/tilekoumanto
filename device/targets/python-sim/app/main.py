import random
import threading
import time

from ffi.core import (
    Core,
    LOG_CB,
    PUBLISH_TELEMETRY_CB,
    READ_TELEMETRY_CB,
    SHOULD_STOP_CB,
    UNIX_TIME_MS_CB,
    Platform,
    Telemetry,
    TK_MAINS_POWER_NOT_PRESENT,
    TK_MAINS_POWER_PRESENT,
    TK_PUMP_RELAY_ACTIVE,
    TK_PUMP_RELAY_INACTIVE,
)


def main() -> int:
    core = Core()
    stop_requested = threading.Event()

    def log(message: bytes) -> None:
        print(f"[python] {message.decode()}")

    def unix_time_ms() -> int:
        return int(time.time() * 1000)

    def read_telemetry(out: Telemetry) -> int:
        out.contents.mains_power = random.choice(
            [
                TK_MAINS_POWER_PRESENT,
                TK_MAINS_POWER_NOT_PRESENT,
            ]
        )

        out.contents.pump_relay = random.choice(
            [
                TK_PUMP_RELAY_ACTIVE,
                TK_PUMP_RELAY_INACTIVE,
            ]
        )

        out.contents.unix_time_ms = unix_time_ms()

        return 1

    def publish_telemetry(telemetry: Telemetry) -> int:
        value = telemetry.contents

        print(
            "[python] publish telemetry "
            f"mains_power={value.mains_power} "
            f"pump_relay={value.pump_relay} "
            f"unix_time_ms={value.unix_time_ms}"
        )

        return 1

    def should_stop() -> int:
        return 1 if stop_requested.is_set() else 0

    log_cb = LOG_CB(log)
    unix_time_ms_cb = UNIX_TIME_MS_CB(unix_time_ms)
    read_telemetry_cb = READ_TELEMETRY_CB(read_telemetry)
    publish_telemetry_cb = PUBLISH_TELEMETRY_CB(publish_telemetry)
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

    return 0


if __name__ == "__main__":
    raise SystemExit(main())