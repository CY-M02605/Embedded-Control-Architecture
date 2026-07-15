cmake_minimum_required(VERSION 3.17)

project(EmbeddedControlArchitecture LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_subdirectory(tests/unit)
add_subdirectory(tests/module/engine_overheat_protection)
add_subdirectory(tests/module/gear_display_facade)
add_subdirectory(tests/module/hydraulic_oil_warning)
add_subdirectory(teste/module/instantiation_practice)
add_subdirectory(wl_coding_fan_control)
add_subdirectory(wl_vehicle_speed)