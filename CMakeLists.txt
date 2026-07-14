cmake_minimum_required(VERSION 3.17)

project(EmbeddedControlArchitecture LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_subdirectory(tests/unit)
add_subdirectory(tests/module/engine_overheat_protection)