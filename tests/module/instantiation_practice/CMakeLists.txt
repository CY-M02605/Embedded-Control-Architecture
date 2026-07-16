cmake_minimum_required(VERSION 3.16)

project(instantiation_practice_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_ROOT_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

enable_testing()

add_executable(
    instantiation_practice_print_based_tests
    print_based_tests.cpp
)

target_include_directories(
    instantiation_practice_print_based_tests
    PRIVATE
    ${PROJECT_ROOT_DIR}/framework
    ${PROJECT_ROOT_DIR}/signals
    ${PROJECT_ROOT_DIR}/utility
    ${PROJECT_ROOT_DIR}/modules/instantiation_practice/include
)

add_test(
    NAME InstantiationPracticeTests
    COMMAND instantiation_practice_print_based_tests
)