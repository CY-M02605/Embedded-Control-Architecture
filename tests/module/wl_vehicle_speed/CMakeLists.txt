cmake_minimum_required(VERSION 3.16)

project(wl_vehicle_speed_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

set(PROJECT_ROOT_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

add_executable(
    wl_vehicle_speed_print_based_tests
    print_based_tests.cpp
    ${PROJECT_ROOT_DIR}/modules/wl_vehicle_speed/src/wl_vehicle_speed.cc
    ${PROJECT_ROOT_DIR}/modules/wl_vehicle_speed/src/wlVehicleSpeed.c
)

target_include_directories(
    wl_vehicle_speed_print_based_tests
    PRIVATE
    ${PROJECT_ROOT_DIR}/framework
    ${PROJECT_ROOT_DIR}/signals
    ${PROJECT_ROOT_DIR}/utility
    ${PROJECT_ROOT_DIR}/modules/wl_vehicle_speed/include
)

add_test(
    NAME WlVehicleSpeedPrintBasedTests
    COMMAND wl_vehicle_speed_print_based_tests
)