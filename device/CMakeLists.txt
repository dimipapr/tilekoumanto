cmake_minimum_required(VERSION 3.16)
project(TilekoumantoCore C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

add_library(tk_core SHARED
    src/tk_core.c
)

target_compile_options(tk_core PRIVATE -Wall -Wextra)