cmake_minimum_required(VERSION 3.10)
project(instantiation_practice LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(instantiation_practice
    main.cpp
)

target_include_directories(instantiation_practice PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../assembly
    ${CMAKE_CURRENT_SOURCE_DIR}/../../framework
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/instantiation_practice/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../signals
)
