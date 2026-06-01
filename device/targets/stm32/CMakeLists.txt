cmake_minimum_required(VERSION 3.22)

project(tilekoumanto_stm32 C ASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(TARGET_NAME tilekoumanto_stm32)

set(STM32_MCU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

add_executable(${TARGET_NAME}.elf
    app/main.c

    third_party/st/Src/system_stm32f4xx.c
    third_party/st/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_gpio.c
    third_party/st/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_utils.c
    third_party/st/startup_stm32f446xx.s
)

target_compile_definitions(${TARGET_NAME}.elf PRIVATE
    STM32F446xx
    USE_FULL_LL_DRIVER
)

target_include_directories(${TARGET_NAME}.elf PRIVATE
    third_party/st/Drivers/CMSIS/Include
    third_party/st/Drivers/CMSIS/Device/ST/STM32F4xx/Include
    third_party/st/Drivers/STM32F4xx_HAL_Driver/Inc
)

target_compile_options(${TARGET_NAME}.elf PRIVATE
    ${STM32_MCU_FLAGS}
    -Wall
    -Wextra
    -Wpedantic
    -ffunction-sections
    -fdata-sections
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