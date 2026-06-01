add_library(tilekoumanto_core
    src/tk_core.c

    third_party/freertos/tasks.c
    third_party/freertos/queue.c
    third_party/freertos/list.c
    third_party/freertos/timers.c
    third_party/freertos/event_groups.c
    third_party/freertos/stream_buffer.c

    third_party/freertos/portable/ThirdParty/GCC/Posix/port.c
    third_party/freertos/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
)

target_include_directories(tilekoumanto_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/freertos/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/freertos/portable/ThirdParty/GCC/Posix
)

target_compile_features(tilekoumanto_core PUBLIC c_std_99)

target_compile_definitions(tilekoumanto_core PRIVATE
    _POSIX_C_SOURCE=200809L
)

target_compile_options(tilekoumanto_core PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)