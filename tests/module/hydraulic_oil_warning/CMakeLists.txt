cmake_minimum_required(VERSION 3.16) # Require CMake 3.10 or higher

project(hydraulic_oil_warning_tests LANGUAGES CXX) # Project name, using C++

set(CMAKE_CXX_STANDARD 11) # Use C++11 standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_ROOT_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

enable_testing()

add_executable(
    hydraulic_oil_warning_print_based_test
    print_based_tests.cpp
    ${PROJECT_ROOT_DIR}/modules/hydraulic_oil_warning/src/hydraulic_oil_warning.cc
)

target_include_directories(
    hydraulic_oil_warning_print_based_test 
    PRIVATE
    ${PROJECT_ROOT_DIR}/framework
    ${PROJECT_ROOT_DIR}/signals
    ${PROJECT_ROOT_DIR}/utility
    ${PROJECT_ROOT_DIR}/module/hydraulic_oil_warning/include
)

add_test(
    NAME HydraulicOilWarningPrintBasedTest
    COMMAND hydraulic_oil_warning_print_based_test
)