cmake_minimum_required(VERSION 3.10)
project(wl_cooling_fan_control LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 11)

add_executable(wl_cooling_fan_control
    src/main.cpp
    src/wl_cooling_fan_control.cc
    src/wlCoolingFanControl.c
)

target_include_directories(wl_cooling_fan_control PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/framework
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/signals
)
