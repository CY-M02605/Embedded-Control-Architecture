cmake_minimum_required(VERSION 3.16)
project(engine_overheat_protection LANGUAGE CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

add_executable(
    engine_overheat_protection
    main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/engine_overheat_protection/src/engine_overheat_protection.cc
)

target_include_directories(
    engine_overheat_protection PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/engine_overheat_protection/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../utility
    ${CMAKE_CURRENT_SOURCE_DIR}/../../framework
    ${CMAKE_CURRENT_SOURCE_DIR}/../../signals
    )