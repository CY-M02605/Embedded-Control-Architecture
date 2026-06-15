cmake_minimum_required(VERSION 3.10)
project(gear_display_facade LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(gear_display_facade
    src/main.cpp
    src/gear_display_facade.cc
)

target_include_directories(gear_display_facade PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
