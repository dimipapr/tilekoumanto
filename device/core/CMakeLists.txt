# device/core/CMakeLists.txt

if(NOT DEFINED TK_FREERTOS_CONFIG_DIR)
    message(FATAL_ERROR "TK_FREERTOS_CONFIG_DIR must be set by the target")
endif()

if(NOT DEFINED TK_FREERTOS_PORT_SOURCES)
    message(FATAL_ERROR "TK_FREERTOS_PORT_SOURCES must be set by the target")
endif()

if(NOT DEFINED TK_FREERTOS_PORT_INCLUDE_DIRS)
    message(FATAL_ERROR "TK_FREERTOS_PORT_INCLUDE_DIRS must be set by the target")
endif()

add_library(freertos_kernel STATIC
    third_party/freertos/tasks.c
    third_party/freertos/queue.c
    third_party/freertos/list.c
    third_party/freertos/timers.c
    third_party/freertos/event_groups.c
    third_party/freertos/stream_buffer.c
    ${TK_FREERTOS_PORT_SOURCES}
)

target_include_directories(freertos_kernel PUBLIC
    ${TK_FREERTOS_CONFIG_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/freertos/include
    ${TK_FREERTOS_PORT_INCLUDE_DIRS}
)

target_compile_features(freertos_kernel PUBLIC c_std_99)

if(DEFINED TK_FREERTOS_COMPILE_DEFINITIONS)
    target_compile_definitions(freertos_kernel PRIVATE
        ${TK_FREERTOS_COMPILE_DEFINITIONS}
    )
endif()

if(DEFINED TK_FREERTOS_COMPILE_OPTIONS)
    target_compile_options(freertos_kernel PRIVATE
        ${TK_FREERTOS_COMPILE_OPTIONS}
    )
    target_compile_options(freertos_kernel PRIVATE
    $<$<C_COMPILER_ID:GNU>:-Wno-unused-variable>
)
endif()

add_library(tilekoumanto_logic STATIC
    src/tk_telemetry.c
)

target_include_directories(tilekoumanto_logic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(tilekoumanto_logic PUBLIC c_std_99)

target_compile_options(tilekoumanto_logic PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror=implicit-function-declaration
    -Werror=incompatible-pointer-types
    -Werror=return-type
    -Werror
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wcast-qual
    -Wwrite-strings
)

if(DEFINED TK_CORE_COMPILE_OPTIONS)
    target_compile_options(tilekoumanto_logic PRIVATE
        ${TK_CORE_COMPILE_OPTIONS}
    )
endif()

add_library(tilekoumanto_core
    src/tk_core.c
    src/tk_telemetry_task.c
    src/tk_status_task.c
)

target_include_directories(tilekoumanto_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(tilekoumanto_core PUBLIC c_std_99)

target_compile_options(tilekoumanto_core PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror=implicit-function-declaration
    -Werror=incompatible-pointer-types
    -Werror=return-type
    -Werror
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wcast-qual
    -Wwrite-strings
)

if(DEFINED TK_CORE_COMPILE_OPTIONS)
    target_compile_options(tilekoumanto_core PRIVATE
        ${TK_CORE_COMPILE_OPTIONS}
    )
endif()

target_link_libraries(
    tilekoumanto_core PRIVATE 
    tilekoumanto_logic
    freertos_kernel
)

include(CTest)

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_executable(test_logic
        tests/test_logic.c
    )

    target_link_libraries(test_logic PRIVATE
        tilekoumanto_logic
    )

    add_test(
        NAME test_logic
        COMMAND test_logic
    )
endif()