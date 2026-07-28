cmake_minimum_required(VERSION 3.17)

project(EmbeddedControlArchitecture LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_subdirectory(tests/unit)
add_subdirectory(tests/module/cooling_fan_control)
add_subdirectory(tests/module/engine_overheat_protection)
add_subdirectory(tests/module/gear_display_facade)
add_subdirectory(tests/module/hydraulic_oil_warning)
add_subdirectory(tests/module/instantiation_practice)
add_subdirectory(tests/module/vehicle_speed_cooling)

add_executable(embedded_control_system
    modules/cooling_fan_control/src/cooling_fan_control.cc
    modules/engine_overheat_protection/src/engine_overheat_protection.cc
    modules/gear_display_facade/src/gear_display_facade.cc
    modules/hydraulic_oil_warning/src/hydraulic_oil_warning.cc
    modules/vehicle_speed_cooling/src/vehicle_speed_cooling.cc
)

target_include_directories(embedded_control_system 
    PUBLIC
        framework
        utility
        signals
        
        modules/cooling_fan_control/include
        modules/engine_overheat_protection/include
        modules/gear_display_facade/include
        modules/hydraulic_oil_warning/include
        modules/vehicle_speed_cooling/include
)