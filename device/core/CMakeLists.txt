add_library(tilekoumanto_core
    src/tk_core.c
)

target_include_directories(tilekoumanto_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(tilekoumanto_core PUBLIC c_std_99)

target_compile_options(tilekoumanto_core PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)