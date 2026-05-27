cmake_minimum_required(VERSION 3.16)
project(TilekoumantoCore C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "-O0")

include_directories(
    include
    third_party/FreeRTOS/Source/include
    third_party/FreeRTOS/Source/portable/ThirdParty/GCC/Posix
    third_party/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils
)

set(CORE_SOURCES
    src/tk_core.c
    third_party/FreeRTOS/Source/tasks.c
    third_party/FreeRTOS/Source/queue.c
    third_party/FreeRTOS/Source/timers.c
    third_party/FreeRTOS/Source/list.c
    third_party/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/port.c
    third_party/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
)

add_library(tk_core SHARED ${CORE_SOURCES})

target_compile_options(tk_core PRIVATE -Wall -Wextra -Wno-unused-parameter)

target_link_libraries(tk_core PRIVATE pthread rt)