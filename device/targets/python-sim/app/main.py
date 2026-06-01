from ffi.core import Core


def main() -> int:
    core = Core()
    print(f"tilekoumanto core version: {core.version()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())