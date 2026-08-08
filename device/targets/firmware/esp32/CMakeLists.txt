cmake_minimum_required(VERSION 3.22)

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/app"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(tilekoumanto_esp32)