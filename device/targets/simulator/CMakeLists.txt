cmake_minimum_required(VERSION 3.22)

project(tilekoumanto_python_target C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS ON)

set(FREERTOS_ROOT
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/third_party/freertos
)

set(TK_FREERTOS_CONFIG_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/config
)

set(TK_FREERTOS_PORT_SOURCES
    ${FREERTOS_ROOT}/portable/ThirdParty/GCC/Posix/port.c
    ${FREERTOS_ROOT}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
)

set(TK_FREERTOS_PORT_INCLUDE_DIRS
    ${FREERTOS_ROOT}/portable/ThirdParty/GCC/Posix
    ${FREERTOS_ROOT}/portable/ThirdParty/GCC/Posix/utils
)

set(TK_FREERTOS_COMPILE_DEFINITIONS
    _POSIX_C_SOURCE=200809L
)

set_source_files_properties(
    ${FREERTOS_ROOT}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
    PROPERTIES
    COMPILE_DEFINITIONS "_GNU_SOURCE"
)

add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core
    ${CMAKE_CURRENT_BINARY_DIR}/core
)

set_target_properties(
    freertos_kernel
    tilekoumanto_logic
    PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_sources(tilekoumanto_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/tk_freertos_hooks.c
)

target_compile_definitions(tilekoumanto_core PRIVATE
    _POSIX_C_SOURCE=200809L
)

set_target_properties(tilekoumanto_core PROPERTIES
    OUTPUT_NAME tilekoumanto_core
)