cmake_minimum_required(VERSION 3.20)

project(EmbeddedControlTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# src directories in Arduino project
set(PROJECT_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Arduino_project/OilTempWarningDemo/src
) 

# start CTest
enable_testing()

# ======================================================================
# 1、 Hysteresis unit test part
# ======================================================================

add_executable(test_hysteresis
    test_hysteresis.cpp
)

target_include_directories(test_hysteresis PRIVATE
    ${PROJECT_SOURCE_DIR}
)

add_test(
    NAME HysteresisTest
    COMMAND test_hysteresis
)


# ======================================================================
# 2、 increment_timer unit test part
# ======================================================================

add_executable(test_increment_timer
    test_increment_timer.cpp
)

target_include_directories(test_increment_timer PRIVATE
    ${PROJECT_SOURCE_DIR}/../../utility
)

add_test(
    NAME IncrementTimerTest
    COMMAND test_increment_time
)