cmake_minimum_required(VERSION 3.22)

project(tilekoumanto_python_target C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS ON)

add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core
    ${CMAKE_CURRENT_BINARY_DIR}/core
)

set_target_properties(tilekoumanto_core PROPERTIES
    OUTPUT_NAME tilekoumanto_core
)