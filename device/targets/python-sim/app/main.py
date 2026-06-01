import time

from ffi.core import Core, LOG_CB, UNIX_TIME_MS_CB, Platform


def main() -> int:
    core = Core()

    def log(message: bytes) -> None:
        print(f"[python] {message.decode()}")

    def unix_time_ms() -> int:
        return int(time.time() * 1000)

    log_cb = LOG_CB(log)
    unix_time_ms_cb = UNIX_TIME_MS_CB(unix_time_ms)

    platform = Platform(
        log=log_cb,
        unix_time_ms=unix_time_ms_cb,
    )

    print(f"tilekoumanto core version: {core.version()}")
    print(f"platform probe: {core.probe_platform(platform)}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())