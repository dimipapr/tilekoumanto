add_library(freertos_kernel STATIC
    third_party/freertos/tasks.c
    third_party/freertos/queue.c
    third_party/freertos/list.c
    third_party/freertos/timers.c
    third_party/freertos/event_groups.c
    third_party/freertos/stream_buffer.c
    third_party/freertos/portable/ThirdParty/GCC/Posix/port.c
    third_party/freertos/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
)

set_target_properties(freertos_kernel PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(freertos_kernel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/freertos/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/freertos/portable/ThirdParty/GCC/Posix
)

target_compile_features(freertos_kernel PUBLIC c_std_99)

target_compile_definitions(freertos_kernel PRIVATE
    _POSIX_C_SOURCE=200809L
)

add_library(tilekoumanto_core
    src/tk_core.c
    src/tk_telemetry.c
)

set_target_properties(tilekoumanto_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
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

target_link_libraries(tilekoumanto_core PRIVATE freertos_kernel)