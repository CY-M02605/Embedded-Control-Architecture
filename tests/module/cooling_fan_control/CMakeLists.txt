cmake_minimum_required(VERSION 3.16)

project(cooling_fan_control_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_ROOT_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

enable_testing()

add_executable(
    cooling_fan_control_print_bansed_tests
    print_based_tests.cpp
    ${PROJECT_ROOT_DIR}/modules/wl_cooling_fan_control/src/cooling_fan_control.cc
    ${PROJECT_ROOT_DIR}/modules/wl_cooling_fan_control/src/coolingFanControl.c
)

target_include_directories(
    cooling_fan_control_print_bansed_tests
    PRIVATE
    ${PROJECT_ROOT_DIR}/framework
    ${PROJECT_ROOT_DIR}/signals
    ${PROJECT_ROOT_DIR}/utility
    ${PROJECT_ROOT_DIR}/modules/cooling_fan_control/include
)

add_test(
    NAME coolingFanControlPrintBasedTests
    COMMAND cooling_fan_control_print_bansed_tests
)