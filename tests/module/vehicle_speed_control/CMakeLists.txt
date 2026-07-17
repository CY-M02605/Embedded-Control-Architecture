cmake_minimum_required(VERSION 3.16)

project(vehicle_speed_control_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

set(PROJECT_ROOT_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

add_executable(
    vehicle_speed_control_print_based_tests
    print_based_tests.cpp
    ${PROJECT_ROOT_DIR}/modules/vehicle_speed_control/src/vehicle_speed_control.cc
    ${PROJECT_ROOT_DIR}/modules/vehicle_speed_control/src/vehicleSpeedControl.c
)

target_include_directories(
    vehicle_speed_control_print_based_tests
    PRIVATE
    ${PROJECT_ROOT_DIR}/framework
    ${PROJECT_ROOT_DIR}/signals
    ${PROJECT_ROOT_DIR}/utility
    ${PROJECT_ROOT_DIR}/modules/vehicle_speed_control/include
)

add_test(
    NAME VehicleSpeedControlPrintBasedTests
    COMMAND vehicle_speed_control_based_tests
)