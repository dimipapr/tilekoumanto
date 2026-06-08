# device/targets/stm32/CMakeLists.txt

cmake_minimum_required(VERSION 3.22)

project(tilekoumanto_stm32 C ASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(TARGET_NAME tilekoumanto_stm32)

set(TK_CORE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../core)
set(FREERTOS_ROOT ${TK_CORE_ROOT}/third_party/freertos)

set(STM32_MCU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

set(TK_FREERTOS_CONFIG_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/config
)

set(TK_FREERTOS_PORT_SOURCES
    ${FREERTOS_ROOT}/portable/GCC/ARM_CM4F/port.c
)

set(TK_FREERTOS_PORT_INCLUDE_DIRS
    ${FREERTOS_ROOT}/portable/GCC/ARM_CM4F
)

set(TK_FREERTOS_COMPILE_OPTIONS
    ${STM32_MCU_FLAGS}
    -Wall
    -Wextra
    -Wpedantic
    -ffunction-sections
    -fdata-sections
)

set(TK_CORE_COMPILE_OPTIONS
    ${STM32_MCU_FLAGS}
    -ffunction-sections
    -fdata-sections
)

add_subdirectory(
    ${TK_CORE_ROOT}
    ${CMAKE_CURRENT_BINARY_DIR}/core
)

add_executable(${TARGET_NAME}.elf
    app/main.c

    platform/system_stm32f4xx.c
    platform/startup_stm32f446xx.s

    third_party/stm32f4xx-hal-driver/Src/stm32f4xx_ll_gpio.c
    third_party/stm32f4xx-hal-driver/Src/stm32f4xx_ll_utils.c
    third_party/stm32f4xx-hal-driver/Src/stm32f4xx_ll_usart.c
    third_party/stm32f4xx-hal-driver/Src/stm32f4xx_ll_rcc.c
)

target_compile_definitions(${TARGET_NAME}.elf PRIVATE
    STM32F446xx
    USE_FULL_LL_DRIVER
)

target_include_directories(${TARGET_NAME}.elf PRIVATE
    third_party/CMSIS_5/CMSIS/Core/Include
    third_party/cmsis-device-f4/Include
    third_party/stm32f4xx-hal-driver/Inc

    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${TK_CORE_ROOT}/config
    ${TK_CORE_ROOT}/include
)

target_compile_options(${TARGET_NAME}.elf PRIVATE
    ${STM32_MCU_FLAGS}
    -Wall
    -Wextra
    -Wpedantic
    -ffunction-sections
    -fdata-sections
    -g
    -Og
)

target_link_libraries(${TARGET_NAME}.elf PRIVATE
    tilekoumanto_core
)

target_link_options(${TARGET_NAME}.elf PRIVATE
    ${STM32_MCU_FLAGS}
    -T${CMAKE_CURRENT_SOURCE_DIR}/linker/STM32F446XX_FLASH.ld
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.map
    --specs=picolibc.specs
)

add_custom_command(TARGET ${TARGET_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${TARGET_NAME}.elf>
    COMMAND ${CMAKE_OBJCOPY} -O binary
        $<TARGET_FILE:${TARGET_NAME}.elf>
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex
        $<TARGET_FILE:${TARGET_NAME}.elf>
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.hex
)