cmake_minimum_required(VERSION 3.16)

project(engine_overheat_protection LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_ROOT_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

add_executable(
    engine_overheat_protection
    main.cpp
    ${PROJECT_ROOT_DIR}/modules/engine_overheat_protection/src/engine_overheat_protection.cc
)

target_include_directories(
    engine_overheat_protection
    PRIVATE
    ${PROJECT_ROOT_DIR}/modules/engine_overheat_protection/include
    ${PROJECT_ROOT_DIR}/utility
    ${PROJECT_ROOT_DIR}/framework
    ${PROJECT_ROOT_DIR}/signals
)