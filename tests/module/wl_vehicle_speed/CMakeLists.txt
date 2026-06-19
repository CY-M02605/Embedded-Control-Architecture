cmake_minimum_required(VERSION 3.10)
project(wl_vehicle_speed LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 11)

add_executable(wl_vehicle_speed
    src/main.cpp
    src/wl_vehicle_speed.cc
    src/wlVehicleSpeed.c
)

target_include_directories(wl_vehicle_speed PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/framework
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/signals
)
