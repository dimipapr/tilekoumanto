cmake_minimum_required(VERSION 3.16)
project(TilekoumantoCore C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "-O0")

set(TARGET_PLATFORM "SIMULATOR" CACHE STRING "Target platform selection: SIMULATOR or EMBEDDED")

#Common Core Sources (Shared by both Simulator and Hardware Target)
set(CORE_SOURCES
    src/tk_core.c
    third_party/FreeRTOS/Source/tasks.c
    third_party/FreeRTOS/Source/queue.c
    third_party/FreeRTOS/Source/timers.c
    third_party/FreeRTOS/Source/list.c
)

# Common Core Includes
set(CORE_INCLUDES
    include
    third_party/FreeRTOS/Source/include
)

# Platform-Specific Branching Logic
if(TARGET_PLATFORM STREQUAL "EMBEDDED")
    message(STATUS "Configuring build for NUCLEO-F446RE EMBEDDED target")
    add_compile_definitions(PLATFORM_EMBEDDED=1)
    
    # Append ARM Cortex-M4F hardware architecture port files
    list(APPEND CORE_SOURCES
        third_party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
        targets/stm32/startup/startup_stm32f446xx.S
    )
    
    # Append ARM Cortex-M4F include path
    list(APPEND CORE_INCLUDES
        third_party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    )
    
    # 1. Name the target "tk_core" (WITHOUT the .elf extension here)
    add_executable(tk_core ${CORE_SOURCES})
    
    # 2. Add an output property so the generated file becomes "tk_core.elf" anyway
    set_target_properties(tk_core PROPERTIES SUFFIX ".elf")
    
    # Now these target configurations will map perfectly to "tk_core"
    target_include_directories(tk_core PRIVATE ${CORE_INCLUDES})
    target_compile_options(tk_core PRIVATE -Wall -Wextra)
    
    # Force the linker to map memory against your NUCLEO script file
    target_link_options(tk_core PRIVATE 
        "-T" "${CMAKE_CURRENT_SOURCE_DIR}/targets/stm32/linker/STM32F446RETx_FLASH.ld"
        "-Wl,--gc-sections"
    )
   # Automatically print memory consumption metrics and exact percentages on successful compilation
    add_custom_command(TARGET tk_core POST_BUILD
        COMMAND ${CMAKE_SIZE} -B tk_core.elf > size_output.txt
        COMMAND python3 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/report_memory.py"
        COMMENT "Generating detailed memory utilization percentages..."
    )
else()
    message(STATUS "Configuring build for Linux SIMULATOR target")
    add_compile_definitions(PLATFORM_SIMULATOR=1)
    
    # Append Linux-specific FreeRTOS files
    list(APPEND CORE_SOURCES
        third_party/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/port.c
        third_party/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
    )
    
    # Append Linux-specific FreeRTOS includes
    list(APPEND CORE_INCLUDES
        third_party/FreeRTOS/Source/portable/ThirdParty/GCC/Posix
        third_party/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils
    )
    
    # Build as a Shared Object library for Python ctypes
    add_library(tk_core SHARED ${CORE_SOURCES})
    target_include_directories(tk_core PRIVATE ${CORE_INCLUDES})
    target_compile_options(tk_core PRIVATE -Wall -Wextra -Wno-unused-parameter)
    target_link_libraries(tk_core PRIVATE pthread rt)
endif()