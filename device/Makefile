.PHONY: all sim stm32 clean rebuild-sim rebuild-stm32

# Default target if you just type 'make'
all: sim stm32

# Incremental Simulator Build
sim:
	@echo "=== Building Simulator (Linux) ==="
	@mkdir -p build_simulator
	@cd build_simulator && cmake -DTARGET_PLATFORM=PLATFORM_SIMULATOR .. && $(MAKE)

# Incremental STM32 Build
stm32:
	@echo "=== Building STM32 (Hardware) ==="
	@mkdir -p build_stm32
	@cd build_stm32 && cmake -DTARGET_PLATFORM=PLATFORM_STM32 -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake .. && $(MAKE)

# Nukes both build folders
clean:
	@echo "=== Cleaning all build directories ==="
	@rm -rf build_simulator/* build_stm32/*
	@echo "Clean complete."

# Force a completely fresh simulator build
rebuild-sim: clean-sim sim
clean-sim:
	@rm -rf build_simulator/*

# Force a completely fresh STM32 build
rebuild-stm32: clean-stm32 stm32
clean-stm32:
	@rm -rf build_stm32/*