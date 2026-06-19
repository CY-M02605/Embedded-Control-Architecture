cmake_minimum_required(VERSION 3.10) # Require CMake 3.10 or higher
project(mini_control_sim_rebuild LANGUAGES CXX) # Project name, using C++
set(CMAKE_CXX_STANDARD 11) # Use C++11 standard

add_executable(mini_control_sim_rebuild
    src/main.cpp
    src/modules/hydraulic_oil_warning.cc
)

target_include_directories(mini_control_sim_rebuild PRIVATE src)