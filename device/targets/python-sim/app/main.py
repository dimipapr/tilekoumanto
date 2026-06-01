import time

from ffi.core import (
    Core,
    LOG_CB,
    READ_TELEMETRY_CB,
    UNIX_TIME_MS_CB,
    Platform,
    Telemetry,
    TK_MAINS_POWER_PRESENT,
    TK_MAINS_POWER_NOT_PRESENT,
    TK_PUMP_RELAY_ACTIVE,
    TK_PUMP_RELAY_INACTIVE
)


def main() -> int:
    core = Core()

    def log(message: bytes) -> None:
        print(f"[python] {message.decode()}")

    def unix_time_ms() -> int:
        return int(time.time() * 1000)

    def read_telemetry(out: Telemetry) -> int:
        out.contents.mains_power = TK_MAINS_POWER_PRESENT
        out.contents.pump_relay = TK_PUMP_RELAY_ACTIVE
        out.contents.unix_time_ms = unix_time_ms()
        return 1

    log_cb = LOG_CB(log)
    unix_time_ms_cb = UNIX_TIME_MS_CB(unix_time_ms)
    read_telemetry_cb = READ_TELEMETRY_CB(read_telemetry)

    platform = Platform(
        log=log_cb,
        unix_time_ms=unix_time_ms_cb,
        read_telemetry=read_telemetry_cb,
    )

    print(f"tilekoumanto core version: {core.version()}")
    print(f"core run: {core.run(platform)}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())