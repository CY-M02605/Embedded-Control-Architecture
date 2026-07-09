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

        bool is_overheat_protected_output;
        signals::ValidityStatus is_overheat_protected_validation;
        float torque_limit_output = 0.0f;
        signals::ValidityStatus torque_limit_validation;
        float fan_request_output = 0.0f;
        signals::ValidityStatus fan_request_validation;
        
    };

    auto create_idle_state_config = CreateDefaultConfig();

    signals::FloatSignal oil_temp(0.0f, signals::ValidityStatus::VALID);
    signals::BoolSignal is_engine_running(false, signals::ValidityStatus::VALID);

    engine_overheat_protection::EngineOverheatProtection eop_test (
        create_idle_state_config,
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

        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_output[i].is_overheat_protected_output);
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_output[i].is_overheat_protected_validation);
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_output[i].torque_limit_output));
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_output[i].torque_limit_validation);
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_output[i].fan_request_output));
        assert(eop_test.FanRequestRef().GetValidity() == scenario_output[i].fan_request_validation);
    }
}

void TestCountingState() {
    framework::Manager manager;

    struct CycleInput {
        float oil_temp;
        signals::ValidityStatus oil_temp_validity;
        bool is_engine_running;
        signals::ValidityStatus is_engine_running_validity;
    };

    struct CycleOutput {
        bool is_overheat_protected_output;
        signals::ValidityStatus is_overheat_protected_validity;
        float torque_limit_output;
        signals::ValidityStatus torque_limit_validity;
        float fan_request_output;
        signals::ValidityStatus fan_request_validity;
    };

    engine_overheat_protection::EngineOverheatProtection::Config create_counting_idle_config = CreateDefaultConfig();

    signals::FloatSignal oil_temp = signals::FloatSignal(0.0f, signals::ValidityStatus::VALID);
    signals::BoolSignal is_engine_running = signals::BoolSignal(0.0f, signals::ValidityStatus::VALID);

    engine_overheat_protection::EngineOverheatProtection eop_test {
        create_counting_idle_config,
        oil_temp,
        is_engine_running,
        manager
    };

    CycleInput scenario_input[] = {

        // if (is_engine_running_.GetValue() && oil_temp_.GetValue() >= config_.oil_high_threshold) -> entrence of COUNTING_1 
        {89.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        // if (!is_engine_running_.IsValid() || !oil_temp_.IsValid())
        {90.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},
        {90.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID},
        {90.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::INVALID},

        // if (is_engine_running_.GetValue() && oil_temp_.GetValue() >= config_.oil_high_threshold) -> entrence of COUNTING_2
        {89.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        // if (!is_engine_running_.GetValue())
        {90.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},

        // if (is_engine_running_.GetValue() && oil_temp_.GetValue() >= config_.oil_high_threshold) -> entrence of COUNTING_3
        {89.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        // if (oil_temp_.GetValue() <= config_.oil_low_threshold)
        {79.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}

    };

    CycleOutput scenario_output[] = {
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 39.8f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 39.8f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 39.8f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID}

    };

    std::size_t size_of_input = sizeof(scenario_input) / sizeof(scenario_input[0]);
    std::size_t size_of_output = sizeof(scenario_output) / sizeof(scenario_output[0]);

    assert(size_of_input == size_of_output);

    for (std::size_t i = 0; i < size_of_input; ++i) {
        oil_temp.SetValue(scenario_input[i].oil_temp);
        oil_temp.SetValidity(scenario_input[i].oil_temp_validity);
        is_engine_running.SetValue(scenario_input[i].is_engine_running);
        is_engine_running.SetValidity(scenario_input[i].is_engine_running_validity);

        manager.UpdateAll();

        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_output[i].is_overheat_protected_output);
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_output[i].is_overheat_protected_validity);
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_output[i].torque_limit_output));
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_output[i].torque_limit_validity);
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_output[i].fan_request_output));
        assert(eop_test.FanRequestRef().GetValidity() == scenario_output[i].fan_request_validity);
    }
};

int main() {
    std::cout << "============ Engine Overheat Protection Tests ============" << std::endl;

    const auto idle_state_start_time = std::chrono::steady_clock::now();

    std::cout << "============ Engine Overheat Protection Idle state Tests Starting ============" << std::endl;

    TestIdleState();

    std::cout << "============ Engine Overheat Protection Idle state Tests Ending ============" << std::endl;

    const auto idle_state_end_time = std::chrono::steady_clock::now();

    const auto idle_state_elapsed_us =
        std::chrono::duration_cast<std::chrono::microseconds>(
            idle_state_end_time - idle_state_start_time
        ).count();

    std::cout << "Idle state test elapsed time: "
              << idle_state_elapsed_us
              << " us"
              << std::endl;

    std::cout << std::endl;

    const auto counting_state_start_time = std::chrono::steady_clock::now();

    std::cout << "============ Engine Overheat Protection Counting state Tests Starting ============" << std::endl;

    TestCountingState();

    std::cout << "============ Engine Overheat Protection Counting state Tests Ending ============" << std::endl;

    const auto counting_state_end_time = std::chrono::steady_clock::now();

    const auto counting_state_elapsed_us =
        std::chrono::duration_cast<std::chrono::microseconds>(
            counting_state_end_time - counting_state_start_time
        ).count();

    std::cout << "Counting state test elapsed time: "
              << counting_state_elapsed_us
              << " us"
              << std::endl;

    std::cout << std::endl;

    return 0;

}
