/**
 * @file assert_based_tests.cpp
 * @brief Provide a PC-based module test for engine overheat protection Arduino version
 * @date 2026-08-04
 */

#include "modules/engine_overheat_protection.h"

#include <iostream>
#include <cmath>
#include <chrono>
#include <cassert>

using Table = utility::LookupTable1D<float>;

bool FloatEqual (float a, float b, float tolerence = 0.001) {
    return std::fabs(a - b) < tolerence;
}

const char* ValidityToStr(signals::ValidityStatus s) {
    return s == signals::ValidityStatus::VALID? "VALID" : "INVALID";
}

const Table::Points oil_temp_fan_request_table[] = {
    {80.0f, 20.0f}, {90.0f, 40.0f}, {100.0f, 65.0f}, {105.0f, 80.0f}, {110.0f, 100.0f}, {115.0f, 100.0f}, {120.0f, 100.0f}
};

const Table::Points oil_temp_torque_limit_table[] = {
    {80.0f, 100.0f}, {90.0f, 100.0f}, {100.0f, 100.0f}, {105.0f, 95.0f}, {110.0f, 80.0f}, {115.0f, 60.0f}, {120.0f, 30.0f}
};

engine_overheat_protection::EngineOverheatProtection::Config CreateDefaultConfig() {

    engine_overheat_protection::EngineOverheatProtection::Config config{};

    config.oil_low_threshold = 80;
    config.oil_high_threshold = 90;
    config.increment_timer_config.threshold_time = 3;
    config.fan_request_lookup_table_points = oil_temp_fan_request_table;
    config.fan_request_lookup_table_size = sizeof(oil_temp_fan_request_table) / sizeof(oil_temp_fan_request_table[0]);
    config.torque_lookup_table_points = oil_temp_torque_limit_table;
    config.torque_lookup_table_size = sizeof(oil_temp_torque_limit_table) / sizeof(oil_temp_torque_limit_table[0]);

    return config;
}

void InvalidTests() {

    framework::Manager manager;

    struct CycleInput {

        float oil_temp_value;
        signals::ValidityStatus oil_temp_validity;
        bool is_engine_running_value;
        signals::ValidityStatus is_engine_running_validity;

    };

    struct CycleOutput {

        bool is_overheat_protection_value;
        signals::ValidityStatus is_overheat_protection_validity;
        float torque_limit_value;
        signals::ValidityStatus torque_limit_validity;
        float fan_request_value;
        signals::ValidityStatus fan_request_validity;

        engine_overheat_protection::EngineOverheatProtectionState state;

    };

    auto create_default_config =  CreateDefaultConfig();

    signals::FloatSignal oil_temp_input = signals::FloatSignal(0.0f, signals::ValidityStatus::INVALID);
    signals::BoolSignal is_engine_running_input = signals::BoolSignal(false, signals::ValidityStatus::INVALID);

    engine_overheat_protection::EngineOverheatProtection eop_test (
        create_default_config,
        oil_temp_input,
        is_engine_running_input,
        manager
    );

    CycleInput scenario_inputs[] = {

        // if (!is_engine_running_.IsValid() || !oil_temp_.IsValid())
        {0.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},
        {0.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID},
        {0.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::INVALID},

        {70.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},
        {70.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID},
        {70.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::INVALID},

        // if (is_engine_running_.GetValue()) && if (oil_temp_.GetValue() < config_.oil_high_threshold)
        {70.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},

        // if (is_engine_running_.GetValue()) && if (oil_temp_.GetValue() >= config_.oil_high_threshold)
        {105.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},

        // if (!is_engine_running_.GetValue()) && if (oil_temp_.GetValue() < config_.oil_high_threshold)
        {70.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},

        // if (!is_engine_running_.GetValue()) && if (oil_temp_.GetValue() < config_.oil_high_threshold)
        {105.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},

    };

    CycleOutput scenario_outputs[] = {
        
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},

        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE},

        {false, signals::ValidityStatus::VALID, 95.0f, signals::ValidityStatus::VALID, 80.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},
        
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 80.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING},
        
    };

    std::size_t size_of_input = sizeof(scenario_inputs) / sizeof(scenario_inputs[0]);
    std::size_t size_of_output = sizeof(scenario_outputs) / sizeof(scenario_outputs[0]);

    assert(size_of_output == size_of_input);

    for (std::size_t i = 0; i < size_of_input; ++i) {
        oil_temp_input.Set(scenario_inputs[i].oil_temp_value, scenario_inputs[i].oil_temp_validity);
        is_engine_running_input.Set(scenario_inputs[i].is_engine_running_value, scenario_inputs[i].is_engine_running_validity);

        manager.UpdateAll();

        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_outputs[i].is_overheat_protection_value);
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_outputs[i].is_overheat_protection_validity);
        // std::cout << eop_test.TorqueLimitRef().GetValue() << " " << scenario_outputs[i].torque_limit_value << std::endl;
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_outputs[i].torque_limit_value));
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_outputs[i].torque_limit_validity);
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_outputs[i].fan_request_value));
        assert(eop_test.FanRequestRef().GetValidity() == scenario_outputs[i].fan_request_validity); 
    }
}

void main() {
    std::cout << "============ Engine Overheat Protection Tests ============" << std::endl;
    std::cout << std::endl;
    const auto stop_state_start_time = std::chrono::steady_clock::now();
    std::cout << "============ Engine Overheat Protection Stop state Tests Starting ============" << std::endl;
    InvalidTests();
    std::cout << "============ Engine Overheat Protection Stop state Tests Ending ============" << std::endl;
    const auto stop_state_end_time = std::chrono::steady_clock::now();
    const auto stop_state_elapsed_us = 
        std::chrono::duration_cast<std::chrono::microseconds> (
            stop_state_end_time - stop_state_start_time
        ).count();
    std::cout << "Stop state test elapsed time: "
              << stop_state_elapsed_us
              << " us"
              << std::endl;
    std::cout << std::endl;
}
