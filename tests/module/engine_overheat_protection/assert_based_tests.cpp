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

const char* ValidityToStr(signals::ValidityStatus s) {
    return s == signals::ValidityStatus::VALID? "VALID" : "INVALID";
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

    engine_overheat_protection::EngineOverheatProtection eop_test (
        create_counting_idle_config,
        oil_temp,
        is_engine_running,
        manager
    );

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
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},

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

void TestProtectedState() {
    framework::Manager manager;

    struct CycleInput {
        float oil_temp;
        signals::ValidityStatus oil_temp_validity;
        float is_engine_running;
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

    engine_overheat_protection::EngineOverheatProtection::Config create_protected_state_config = CreateDefaultConfig();

    signals::FloatSignal oil_temp = signals::FloatSignal(0.0f, signals::ValidityStatus::VALID);
    signals::BoolSignal is_engine_running = signals::BoolSignal(false, signals::ValidityStatus::VALID);

    engine_overheat_protection::EngineOverheatProtection eop_test (
        create_protected_state_config,
        oil_temp,
        is_engine_running,
        manager
    );

    CycleInput scenario_input[] = {

        // if (increment_timer_.IsTimeUp()) -> entrence of PROTECTED_1
        {89.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.2f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.3f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        // if (!is_engine_running_.IsValid() || !oil_temp_.IsValid())
        {90.3f, signals::ValidityStatus::INVALID, true, signals::ValidityStatus::VALID},
        {90.3f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::INVALID},
        {90.3f, signals::ValidityStatus::INVALID, true, signals::ValidityStatus::INVALID},

        // if (increment_timer_.IsTimeUp()) -> entrence of PROTECTED_1
        {89.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.2f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.3f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        // if (oil_temp_.GetValue() <= config_.oil_low_threshold)
        {79.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}

    };

    CycleOutput scenario_output[] = {
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 39.8f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.25f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.5f, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.75f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},
        
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 39.8f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.25f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.5f, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.75f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID}
    };

    std::size_t size_of_input = sizeof(scenario_input) / sizeof(scenario_input[0]);
    std::size_t size_of_output = sizeof(scenario_output) / sizeof(scenario_output[0]);

    assert(size_of_output == size_of_input);

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
}

void TestAfterRunCoolingState() {
    framework::Manager manager;

    engine_overheat_protection::EngineOverheatProtection::Config create_after_run_cooling_state_config = CreateDefaultConfig();

    signals::FloatSignal oil_temp = signals::FloatSignal(0.0f, signals::ValidityStatus::INVALID);
    signals::BoolSignal is_engine_running = signals::BoolSignal(false, signals::ValidityStatus::INVALID);

    engine_overheat_protection::EngineOverheatProtection eop_test (
        create_after_run_cooling_state_config,
        oil_temp,
        is_engine_running,
        manager
    );
    
    struct CycelInput {
        float oil_temp;
        signals::ValidityStatus oil_temp_validity;
        bool is_engine_running;
        signals::ValidityStatus is_engine_running_validity;
    };

    struct CycleOutput {
        bool is_overheat_protecte_output;
        signals::ValidityStatus is_overheat_protected_validity;
        float torque_limit;
        signals::ValidityStatus torque_limit_validity;
        float fan_request_output;
        signals::ValidityStatus fan_request_validity;
    };

    CycelInput scenario_input[] = {
        // entrance of after run cooling 1
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.2f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.3f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.4f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},
        // if (!is_engine_running_.IsValid() || !oil_temp_.IsValid())
        {80.0f, signals::ValidityStatus::INVALID, true, signals::ValidityStatus::VALID},
        {80.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::INVALID},
        {80.2f, signals::ValidityStatus::INVALID, true, signals::ValidityStatus::INVALID},
        
        // entrance of after run cooling 2
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.2f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.3f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.4f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},
        // if (oil_temp_.GetValue() <= config_.oil_low_threshold)
        {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},

        // entrance of after run cooling 3
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.2f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.3f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.4f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},
        {90.5f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},
        // if (is_engine_running_.GetValue() && oil_temp_.GetValue() >= config_.oil_high_threshold && !increment_timer_.IsTimeUp())
        {90.6f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.7f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        // if (increment_timer_.IsTimeUp())
        {90.8f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
    };

    CycleOutput scenario_output[] = {
        // entrance of after run cooling 1
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.25f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.5f, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.75f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 41.0f, signals::ValidityStatus::VALID},
        // if (!is_engine_running_.IsValid() || !oil_temp_.IsValid())
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID},

        // entrance of after run cooling 2
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.25f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.5f, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.75f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 41.0f, signals::ValidityStatus::VALID},
        // if (oil_temp_.GetValue() <= config_.oil_low_threshold)
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID},

        // entrance of after run cooling 3
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.25f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.5f, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.75f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 41.0f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 41.25f, signals::ValidityStatus::VALID},
        // if (is_engine_running_.GetValue() && oil_temp_.GetValue() >= config_.oil_high_threshold && !increment_timer_.IsTimeUp())
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 41.5f, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 41.75f, signals::ValidityStatus::VALID},
        // if (increment_timer_.IsTimeUp())
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 42.0f, signals::ValidityStatus::VALID},
    };

    size_t size_of_input = sizeof(scenario_input) / sizeof(scenario_input[0]);
    size_t size_of_output = sizeof(scenario_output) / sizeof(scenario_output[0]);

    assert(size_of_input == size_of_output);

    for (size_t i = 0; i < size_of_input; ++i) {
        oil_temp.SetValue(scenario_input[i].oil_temp);
        oil_temp.SetValidity(scenario_input[i].oil_temp_validity);
        is_engine_running.SetValue(scenario_input[i].is_engine_running);
        is_engine_running.SetValidity(scenario_input[i].is_engine_running_validity);

        manager.UpdateAll();

        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_output[i].is_overheat_protecte_output);
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_output[i].is_overheat_protected_validity);
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_output[i].fan_request_output));
        assert(eop_test.FanRequestRef().GetValidity() == scenario_output[i].fan_request_validity);
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_output[i].torque_limit));
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_output[i].fan_request_validity);

    }

}

void TestFromIdleToProtected() {
    framework::Manager manager;

    struct CycleInput {
        float oil_temp;
        signals::ValidityStatus oil_temp_validity;
        bool is_engine_running;
        signals::ValidityStatus is_engine_running_validity;
    };

    struct CycleOutput {
        bool is_engine_overheat_protected_output;
        signals::ValidityStatus is_engine_overheat_protected_validity;
        float torque_limit_output;
        signals::ValidityStatus torque_limit_validity;
        float fan_request_output;
        signals::ValidityStatus fan_request_validity;
    };

    engine_overheat_protection::EngineOverheatProtection::Config create_all_states_config = CreateDefaultConfig();

    signals::FloatSignal oil_temp = signals::FloatSignal(0.0f, signals::ValidityStatus::VALID);
    signals::BoolSignal is_engine_running = signals::BoolSignal(true, signals::ValidityStatus::VALID);

    engine_overheat_protection::EngineOverheatProtection eop_test (
      create_all_states_config,
      oil_temp,
      is_engine_running,
      manager  
    );

    CycleInput scenario_input[] = {
        {30.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},
        {40.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {50.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {60.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {70.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {89.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {95.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {100.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {102.5f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {105.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {107.5f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {110.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {112.5f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {115.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {117.5f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {120.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {121.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {118.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {113.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {107.2f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {102.8f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {97.8f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {94.3f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {84.8f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {79.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {64.2f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {58.5f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},
        {44.7f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},
        {36.3f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},
    };

    CycleOutput scenario_output[] = {
        // No.1 - {30.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::INVALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID},
        // No.2 - {40.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID},
        // No.3 - {50.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID},
        // No.4 - {60.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID},
        // No.5 - {70.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID},
        // No.6 - {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID},
        // No.7 - {89.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 39.8f, signals::ValidityStatus::VALID},
        // No.8 - {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},
        // No.9 - {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},
        // No.10 - {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},
        // No.11 - {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID},
        // No.12 - {95.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 52.5f, signals::ValidityStatus::VALID},
        // No.13 - {100.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 65.0f, signals::ValidityStatus::VALID},
        // No.14 - {102.5f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 97.5f, signals::ValidityStatus::VALID, 72.5f, signals::ValidityStatus::VALID},
        // No.15 - {105.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 95.0f, signals::ValidityStatus::VALID, 80.0f, signals::ValidityStatus::VALID},
        // No.16 - {107.5f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 87.5f, signals::ValidityStatus::VALID, 90.0f, signals::ValidityStatus::VALID},
        // No.17 - {110.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 80.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID},
        // No.18 - {112.5f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 70.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID},
        // No.19 - {115.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 60.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID},
        // No.20 - {117.5f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 45.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID},
        // No.21 - {120.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 30.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID},
        // No.22 - {121.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 30.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID},
        // No.23 - {118.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 41.4f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID},
        // No.24 - {113.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 64.4f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID},
        // No.25 - {107.2f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 88.4f, signals::ValidityStatus::VALID, 88.8f, signals::ValidityStatus::VALID},
        // No.26 - {102.8f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 97.2f, signals::ValidityStatus::VALID, 73.4f, signals::ValidityStatus::VALID},
        // No.27 - {97.8f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 59.5f, signals::ValidityStatus::VALID},
        // No.28 - {94.3f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 50.75f, signals::ValidityStatus::VALID},
        // No.29 - {84.8f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 29.6f, signals::ValidityStatus::VALID},
        // No.30 - {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID},
        // No.31 - {79.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID},
        // No.32 - {64.2f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID},
        // No.33 - {58.5f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID},
        // No.34 - {44.7f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID},
        // No.35 - {36.3f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID},
    };

    std::size_t size_of_input = sizeof(scenario_input) / sizeof(scenario_input[0]);
    std::size_t size_of_output = sizeof(scenario_output) / sizeof(scenario_output[0]);

    assert(size_of_output == size_of_input);

    for (std::size_t i = 0; i < size_of_input; ++i) {
        oil_temp.SetValue(scenario_input[i].oil_temp);
        oil_temp.SetValidity(scenario_input[i].oil_temp_validity);
        is_engine_running.SetValue(scenario_input[i].is_engine_running);
        is_engine_running.SetValidity(scenario_input[i].is_engine_running_validity);

        manager.UpdateAll();

        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_output[i].is_engine_overheat_protected_output);
        // std::cout << ValidityToStr(eop_test.IsOverheatProtectedRef().GetValidity()) << std::endl;
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_output[i].is_engine_overheat_protected_validity);
        // std::cout << "No." << i + 1 << ": Torque limit value - " << eop_test.TorqueLimitRef().GetValue() << std::endl;
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_output[i].torque_limit_output));
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_output[i].torque_limit_validity);
        // std::cout << "No." << i + 1 << ": Fan request value - " << eop_test.FanRequestRef().GetValue() << std::endl;
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_output[i].fan_request_output));
        assert(eop_test.FanRequestRef().GetValidity() == scenario_output[i].fan_request_validity);
    }

}

int main() {
    std::cout << "============ Engine Overheat Protection Tests ============" << std::endl;

    std::cout << std::endl;

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

    std::cout << "============ Engine Overheat Protection Counting State Tests Starting ============" << std::endl;

    TestCountingState();

    std::cout << "============ Engine Overheat Protection Counting State Tests Ending ============" << std::endl;

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

    const auto protected_state_start_time = std::chrono::steady_clock::now();

    std::cout << "============ Engine Overheat Protection Protected State Tests Starting ============" << std::endl;

    TestProtectedState();

    std::cout << "============ Engine Overheat Protection Protected State Tests Ending ============" << std::endl;

    const auto protected_state_end_time = std::chrono::steady_clock::now();

    const auto protected_state_elapsed_us =
        std::chrono::duration_cast<std::chrono::microseconds>(
            protected_state_end_time - protected_state_start_time
        ).count();

    std::cout << "Protected state test elapsed time: "
              << protected_state_elapsed_us
              << " us"
              << std::endl;

    std::cout << std::endl;

    const auto after_run_cooling_state_start_time = std::chrono::steady_clock::now();

    std::cout << "============ Engine Overheat Protection After Run Cooling State Tests Starting ============" << std::endl;

    TestAfterRunCoolingState();

    std::cout << "============ Engine Overheat Protection After Run Cooling State Tests Ending ============" << std::endl;

    const auto after_run_cooling_state_end_time = std::chrono::steady_clock::now();

    const auto after_run_cooling_state_elapsed_us = 
        std::chrono::duration_cast<std::chrono::microseconds>(
            after_run_cooling_state_end_time - after_run_cooling_state_start_time
        ).count();

    std::cout << "All states test elapsed time: "
              << after_run_cooling_state_elapsed_us
              << " us"
              << std::endl;

    std::cout << std::endl;

    const auto all_states_start_time = std::chrono::steady_clock::now();

    std::cout << "============ Engine Overheat Protection All States Tests Starting ============" << std::endl;

    TestFromIdleToProtected();

    std::cout << "============ Engine Overheat Protection All States Tests Ending ============" << std::endl;

    const auto all_states_end_time = std::chrono::steady_clock::now();

    const auto all_states_elapsed_us = 
        std::chrono::duration_cast<std::chrono::microseconds>(
            all_states_end_time - all_states_start_time
        ).count();

    std::cout << "All states test elapsed time: "
              << all_states_elapsed_us
              << " us"
              << std::endl;
            
    std::cout << std::endl;

    return 0;

}
