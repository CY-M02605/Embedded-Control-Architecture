cmake_minimum_require(VERSION 3.16)

project(engine_overheat_protection_Arduino_tests LANGUAGE CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_ROOT_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

add_executable(
    engine_overheat_protection_Arduino_assert_based_test
    assert_based_tests.cpp
    ${PROJECT_ROOT_DIR}/Arduino_project/EngineOverheatProtection/src/modules/engine_overheat_protection.cc
)

target_include_directories(
    engine_overheat_protection_Arduino_assert_based_test
    PRIVATE
    ${PROJECT_ROOT_DIR}/Arduino_project/EngineOverheatProtection/modules
    ${PROJECT_ROOT_DIR}/Arduino_project/EngineOverheatProtection/src/framework
    ${PROJECT_ROOT_DIR}/Arduino_project/EngineOverheatProtection/src/signals
    ${PROJECT_ROOT_DIR}/Arduino_project/EngineOverheatProtection/src/utility
)

enable_testing()

add_test(
    NAME engine_overheat_protection_Arduino_assert_based_test
    COMMAND engine_overheat_protection_Arduino_assert_based_test
)