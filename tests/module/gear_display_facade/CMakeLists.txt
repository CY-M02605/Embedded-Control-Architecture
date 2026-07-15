cmake_minimum_required(VERSION 3.16)
project(gear_display_facade_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_ROOT_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

add_executable(
    gear_display_facade_tests
    print_based_tests.cpp
    ${PROJECT_ROOT_DIR}/modules/gear_display_facade/src/gear_display_facade.cc
)

target_include_directories(
    gear_display_facade_tests 
    PRIVATE
    ${PROJECT_ROOT_DIR}/modules/gear_display_facade/include
    ${CMAKE_CURRENT_SOURCE_DIR}//framework
    ${CMAKE_CURRENT_SOURCE_DIR}/signals
    ${CMAKE_CURRENT_SOURCE_DIR}/utility
)
