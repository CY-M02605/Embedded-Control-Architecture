/**
 * @file main.cpp (for engine_overheat_protection.cpp)
 * @brief Provides a PC-based module test for the EngineOverheatProtection module.
 * @date 2026-07-03
 */

#include "engine_overheat_protection.h"

#include <iostream>
#include <cmath>
#include <cassert>
#include <chrono>

bool FloatEqual(float a, float b, float tolerance = 0.001f) {
    return std::fabs(a - b) < tolerance;
}

using Table = utility::LookupTable1D<float>;

const Table::Point oil_temp_fan_request_table[] = {
    {80.0f, 20.0f}, {90.0f, 40.0f}, {100.0f, 65.0f}, {105.0f, 80.0f}, {110.0f, 100.0f}, {115.0f, 100.0f}, {120.0f, 100.0f}
};

const Table::Point oil_temp_torque_limit_table[] = {
    {80.0f, 100.0f}, {90.0f, 100.0f}, {100.0f, 100.0f}, {105.0f, 95.0f}, {110.0f, 80.0f}, {115.0f, 60.0f}, {120.0f, 30.0f}
};

engine_overheat_protection::EngineOverheatProtection::Config CreateDefaultConfig() {
    
    engine_overheat_protection::EngineOverheatProtection::Config config{};

    config.oil_low_threshold = 80;
    config.oil_high_threshold = 90;
    config.increment_timer_config.threshold_timer = 3;
    config.fan_lookup_table_points = oil_temp_fan_request_table;
    config.fan_lookup_table_size = sizeof(oil_temp_fan_request_table) / sizeof(oil_temp_fan_request_table[0]);
    config.torque_lookup_table_points = oil_temp_torque_limit_table;
    config.torque_lookup_table_size = sizeof(oil_temp_torque_limit_table) / sizeof(oil_temp_torque_limit_table[0]);

    return config;
}

void TestIdleState() {

    framework::Manager manager;

    struct CycleInput {

        float oil_temp;
        signals::ValidityStatus oil_temp_validity;
        bool is_engine_running;
        signals::ValidityStatus is_engine_running_validity;
    
    };

    struct CycleOutput {

        bool alarm_out;
        signals::ValidityStatus alarm_validation;
        float t_d_output = 0.0f;
        signals::ValidityStatus t_d_validation;
        float fan_output = 0.0f;
        signals::ValidityStatus fan_validation;
        
    };

    auto create_default_config = CreateDefaultConfig();

    signals::FloatSignal oil_temp(0.0f, signals::ValidityStatus::VALID);
    signals::BoolSignal is_engine_running(false, signals::ValidityStatus::VALID);

    engine_overheat_protection::EngineOverheatProtection eop_test (
        create_default_config,
        oil_temp,
        is_engine_running,
        manager
    );

    CycleInput scenario_input[] = {

        // if (!is_engine_running_.IsValid() || !oil_temp_.IsValid())
        {0.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},
        {0.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID},
        {0.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::INVALID},

        {40.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},
        {40.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID},
        {40.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::INVALID},

        // if (!is_engine_running_.GetValue() && oil_temp_.GetValue() < config_.oil_high_threshold)
        {89.9f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},

        // if (!is_engine_running_.GetValue() && oil_temp_.GetValue() >= config_.oil_high_threshold)
        {90.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},

        // if (is_engine_running_.GetValue() && oil_temp_.GetValue() < config_.oil_high_threshold)
        {89.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}

    };

    CycleOutput scenario_output[] = {

        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},

        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},
        
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 39.8f, signals::ValidityStatus::VALID},
        
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 39.8f, signals::ValidityStatus::VALID},

    };

    std::size_t size_of_input = sizeof(scenario_input) / sizeof(scenario_input[0]);
    std::size_t size_of_output = sizeof(scenario_output) / sizeof(scenario_output[0]);

    assert(size_of_input == size_of_output);


    for (std::size_t i = 0; i < sizeof(scenario_input) / sizeof(scenario_input[0]); ++i) {

        oil_temp.SetValue(scenario_input[i].oil_temp);
        oil_temp.SetValidity(scenario_input[i].oil_temp_validity);
        is_engine_running.SetValue(scenario_input[i].is_engine_running);
        is_engine_running.SetValidity(scenario_input[i].is_engine_running_validity);

        manager.UpdateAll();

        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_output[i].alarm_out);
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_output[i].alarm_validation);
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_output[i].t_d_output));
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_output[i].t_d_validation);
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_output[i].fan_output));
        assert(eop_test.FanRequestRef().GetValidity() == scenario_output[i].fan_validation);
    }
}

void TestEnterCountingWhenTemperatureHigh();

void TestEnterProtectedAfterTimer();

void TestReturnToIdleWhenTemperatureLow();

int main() {
    std::cout << "============ Engine Overheat Protection Tests ============" << std::endl;

    const auto idle_state_start_time = std::chrono::steady_clock::now();

    std::cout << "============ Engine Overheat Protection Idle state Tests Starting ============" << std::endl;

    TestIdleState();

    std::cout << "============ Engine Overheat Protection Idle state Tests Ending ============" << std::endl;

    const auto idle_state_end_time = std::chrono::steady_clock::now();

    const auto idle_state_elapsed_ms =
        std::chrono::duration_cast<std::chrono::microseconds>(
            idle_state_end_time - idle_state_start_time
        ).count();

    std::cout << "Test elapsed time: "
              << idle_state_elapsed_ms
              << " us"
              << std::endl;

    return 0;
}
