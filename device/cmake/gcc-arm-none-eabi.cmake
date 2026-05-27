set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Specify the cross-compiler tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_SIZE arm-none-eabi-size)

# Target-specific hardware compilation flags (adjust for your exact STM32 MCU core, e.g., Cortex-M4)
set(OBJECT_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")

set(CMAKE_C_FLAGS "${OBJECT_FLAGS} -Wall -Wextra -fdata-sections -ffunction-sections" CACHE INTERNAL "C Compiler Flags")
set(CMAKE_ASM_FLAGS "${OBJECT_FLAGS} -Wall -x assembler-with-cpp" CACHE INTERNAL "ASM Compiler Flags")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections" CACHE INTERNAL "Linker Flags")