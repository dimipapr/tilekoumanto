cmake_minimum_required(VERSION 3.16)
project(TilekoumantoCore C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "-O0")

set(TARGET_PLATFORM "PLATFORM_SIMULATOR" CACHE STRING "Target platform selection")

# Common Sources and Includes
set(CORE_SOURCES
    src/tk_core.c
    third_party/FreeRTOS/Source/tasks.c
    third_party/FreeRTOS/Source/queue.c
    third_party/FreeRTOS/Source/timers.c
    third_party/FreeRTOS/Source/list.c
)

set(CORE_INCLUDES
    include
    third_party/FreeRTOS/Source/include
)

# STM32 Specific Logic
if(TARGET_PLATFORM STREQUAL "PLATFORM_STM32")
    # Define startup file separately to ensure strict ordering
    set(STARTUP_FILE targets/stm32/startup/startup_stm32f446xx.S)
    
    # Add files
    list(APPEND CORE_SOURCES
        third_party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
        targets/stm32/Core/Src/main.c
        third_party/stm32_ll/system_stm32f4xx.c
        third_party/stm32_ll/stm32f4xx_ll_dma.c
        third_party/stm32_ll/stm32f4xx_ll_exti.c
        third_party/stm32_ll/stm32f4xx_ll_gpio.c
        third_party/stm32_ll/stm32f4xx_ll_pwr.c
        third_party/stm32_ll/stm32f4xx_ll_rcc.c
        third_party/stm32_ll/stm32f4xx_ll_utils.c
    )

    # Create Executable with explicit Startup File priority
    add_executable(tk_core ${STARTUP_FILE} ${CORE_SOURCES})
    set_target_properties(tk_core PROPERTIES SUFFIX ".elf")
    
    # Definitions and Linking
    add_compile_definitions(PLATFORM_STM32=1 STM32F446xx)
    list(APPEND CORE_INCLUDES third_party/FreeRTOS/Source/portable/GCC/ARM_CM4F third_party/stm32_ll)
    
    target_link_options(tk_core PRIVATE 
        "-T" "${CMAKE_CURRENT_SOURCE_DIR}/targets/stm32/linker/STM32F446RETx_FLASH.ld"
        "-Wl,--gc-sections"
        "-Wl,--entry=Reset_Handler"
    )
    
    add_custom_command(TARGET tk_core POST_BUILD
        COMMAND ${CMAKE_SIZE} -B tk_core.elf > size_output.txt
        COMMAND python3 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/report_memory.py"
    )

# Simulator Specific Logic
else()
    # Append the simulator specific source files before add_library
    list(APPEND CORE_SOURCES
        third_party/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/port.c
        third_party/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
        targets/simulator/src/sim_hooks.c
    )
    list(APPEND CORE_INCLUDES
        third_party/FreeRTOS/Source/portable/ThirdParty/GCC/Posix
        third_party/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils
    )
    add_compile_definitions(PLATFORM_SIMULATOR=1)
    
    add_library(tk_core SHARED ${CORE_SOURCES})
    target_link_libraries(tk_core PRIVATE pthread rt)
endif()

# Global Configuration
target_include_directories(tk_core PRIVATE ${CORE_INCLUDES})
target_compile_options(tk_core PRIVATE -Wall -Wextra)
if(NOT TARGET_PLATFORM STREQUAL "PLATFORM_STM32")
    target_compile_options(tk_core PRIVATE -Wno-unused-parameter)
endif()