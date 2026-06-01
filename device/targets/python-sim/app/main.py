from ffi.core import (
    Core,
    Telemetry,
    TK_MAINS_POWER_NOT_PRESENT,
    TK_MAINS_POWER_PRESENT,
    TK_PUMP_RELAY_INACTIVE,
)


def main() -> int:
    core = Core()

    previous = Telemetry(
        mains_power=TK_MAINS_POWER_PRESENT,
        pump_relay=TK_PUMP_RELAY_INACTIVE,
        unix_time_ms=1000,
    )

    same = Telemetry(
        mains_power=TK_MAINS_POWER_PRESENT,
        pump_relay=TK_PUMP_RELAY_INACTIVE,
        unix_time_ms=2000,
    )

    changed = Telemetry(
        mains_power=TK_MAINS_POWER_NOT_PRESENT,
        pump_relay=TK_PUMP_RELAY_INACTIVE,
        unix_time_ms=3000,
    )

    print(f"tilekoumanto core version: {core.version()}")
    print(f"first sample publishes: {core.should_publish_telemetry(None, previous)}")
    print(f"same state publishes: {core.should_publish_telemetry(previous, same)}")
    print(f"changed state publishes: {core.should_publish_telemetry(previous, changed)}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())