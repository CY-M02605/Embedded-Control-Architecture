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

const char* FaultReasonToStr(engine_overheat_protection::FaultReason s) {
    switch (s)
    {
    case engine_overheat_protection::FaultReason::NONE:
        return "NONE";
    case engine_overheat_protection::FaultReason::ENGINE_RUNNING_SIGNAL_INVALID:
        return "ENGINE_RUNNING_SIGNAL_INVALID";
    case engine_overheat_protection::FaultReason::OIL_TEMP_SIGNAL_INVALID:
        return "OIL_TEMP_SIGNAL_INVALID";
    case engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH:
        return "TEMP_OUT_OF_RANGE_HIGH";
    case engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW:
        return "TEMP_OUT_OF_RANGE_LOW";
    case engine_overheat_protection::FaultReason::UNEXPECTED_HIGH_TEMP_IN_STOP:
        return "UNEXPECTED_HIGH_TEMP_IN_STOP";
    default:
        return "0";
    }
}

const char* StateToSte(engine_overheat_protection::EngineOverheatProtectionState s) {
    switch (s)
    {
    case engine_overheat_protection::EngineOverheatProtectionState::STOP:
        return "STOP";
    case engine_overheat_protection::EngineOverheatProtectionState::IDLE:
        return "IDLE";
    case engine_overheat_protection::EngineOverheatProtectionState::COUNTING:
        return "COUNTING";
    case engine_overheat_protection::EngineOverheatProtectionState::PROTECTED:
        return "PROTECTED";
    case engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING:
        return "AFTER_RUN_COOLING";
    case engine_overheat_protection::EngineOverheatProtectionState::FAULT:
        return "FAULT";
    default:
        return "0";
    }
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

void StopStateTest() {

    framework::Manager manager;

    struct CycleInput {

        float oil_temp_value;
        signals::ValidityStatus oil_temp_validity;
        bool is_engine_running_value;
        signals::ValidityStatus is_engine_running_validity;
        bool clear_fault_request_value;
        signals::ValidityStatus clear_fault_request_validity;

    };

    struct CycleOutput {

        bool is_overheat_protection_value;
        signals::ValidityStatus is_overheat_protection_validity;
        float torque_limit_value;
        signals::ValidityStatus torque_limit_validity;
        float fan_request_value;
        signals::ValidityStatus fan_request_validity;

        engine_overheat_protection::EngineOverheatProtectionState state;
        engine_overheat_protection::FaultReason fault_reason;

    };

    auto create_default_config =  CreateDefaultConfig();

    signals::FloatSignal oil_temp_input = signals::FloatSignal(0.0f, signals::ValidityStatus::INVALID);
    signals::BoolSignal is_engine_running_input = signals::BoolSignal(false, signals::ValidityStatus::INVALID);
    signals::BoolSignal clear_fault_request_input = signals::BoolSignal(false, signals::ValidityStatus::INVALID);

    engine_overheat_protection::EngineOverheatProtection eop_test (
        create_default_config,
        oil_temp_input,
        is_engine_running_input,
        clear_fault_request_input,
        manager
    );

    CycleInput scenario_inputs[] = {

        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP
        {78.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // FAULT: OIL_TEMP_SIGNAL_INVALID
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // STOP (recover)

        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},   // FAULT: ENGINE_RUNNING_SIGNAL_INVALID
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // STOP (recover)

        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP
        {150.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},  // FAULT: TEMP_OUT_OF_RANGE_HIGH
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // STOP (recover)
        
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP
        {90.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // FAULT: UNEXPECTED_HIGH_TEMP_IN_STOP
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // STOP (recover)

        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP
        {-20.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},  // FAULT: TEMP_OUT_OF_RANGE_LOW
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // STOP (recover)

        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP
        {78.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // IDLE

    };

    CycleOutput scenario_outputs[] = {
        
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::OIL_TEMP_SIGNAL_INVALID}, 
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::ENGINE_RUNNING_SIGNAL_INVALID}, 
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH}, 
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::UNEXPECTED_HIGH_TEMP_IN_STOP}, 
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW}, 
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE}
        
    };

    std::size_t size_of_input = sizeof(scenario_inputs) / sizeof(scenario_inputs[0]);
    std::size_t size_of_output = sizeof(scenario_outputs) / sizeof(scenario_outputs[0]);

    assert(size_of_output == size_of_input);

    for (std::size_t i = 0; i < size_of_input; ++i) {
        oil_temp_input.Set(scenario_inputs[i].oil_temp_value, scenario_inputs[i].oil_temp_validity);
        is_engine_running_input.Set(scenario_inputs[i].is_engine_running_value, scenario_inputs[i].is_engine_running_validity);
        clear_fault_request_input.Set(scenario_inputs[i].clear_fault_request_value, scenario_inputs[i].clear_fault_request_validity);

        manager.UpdateAll();

        // std::cout << "No." << i+1 << std::endl;
        // std::cout << eop_test.IsOverheatProtectedRef().GetValue()  << " " << scenario_outputs[i].is_overheat_protection_value << std::endl;
        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_outputs[i].is_overheat_protection_value);
        // std::cout << ValidityToStr(eop_test.IsOverheatProtectedRef().GetValidity())  << " " << ValidityToStr(scenario_outputs[i].is_overheat_protection_validity) << std::endl;
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_outputs[i].is_overheat_protection_validity);
        // std::cout << eop_test.TorqueLimitRef().GetValue() << " " << scenario_outputs[i].torque_limit_value << std::endl;
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_outputs[i].torque_limit_value));
        // std::cout << ValidityToStr(eop_test.TorqueLimitRef().GetValidity())  << " " << ValidityToStr(scenario_outputs[i].torque_limit_validity) << std::endl;
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_outputs[i].torque_limit_validity);
        // std::cout << eop_test.FanRequestRef().GetValue()  << " " << scenario_outputs[i].fan_request_value << std::endl;
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_outputs[i].fan_request_value));
        // std::cout << ValidityToStr(eop_test.FanRequestRef().GetValidity())  << " " << ValidityToStr(scenario_outputs[i].fan_request_validity) << std::endl;
        assert(eop_test.FanRequestRef().GetValidity() == scenario_outputs[i].fan_request_validity); 

        // std::cout << StateToSte(eop_test.StateRef())  << " " << StateToSte(scenario_outputs[i].state) << std::endl;
        assert(eop_test.StateRef() == scenario_outputs[i].state);
        // std::cout << FaultReasonToStr(eop_test.FaultReasonRef())  << " " << FaultReasonToStr(scenario_outputs[i].fault_reason) << std::endl;
        assert(eop_test.FaultReasonRef() == scenario_outputs[i].fault_reason);
    }
}

void IdleStateTest() {
    framework::Manager manager;

    struct CycleInput {
        float oil_temp_value;
        signals::ValidityStatus oil_temp_validity;
        bool is_engine_running_value;
        signals::ValidityStatus is_engine_running_validity; 
        bool clear_fault_request_value;
        signals::ValidityStatus clear_fault_request_validity;
    };

    struct CycleOutput {
        bool is_overheat_protection_output;
        signals::ValidityStatus is_overheat_protection_validity;

        float torque_limit_output;
        signals::ValidityStatus torque_limit_validity;

        float fan_request_output;
        signals::ValidityStatus fan_request_validity;

        engine_overheat_protection::EngineOverheatProtectionState state_;
        engine_overheat_protection::FaultReason fault_reason_;
    };

    auto create_default_config = CreateDefaultConfig();

    signals::FloatSignal oil_temp_input = signals::FloatSignal(0.0f, signals::ValidityStatus::INVALID);
    signals::BoolSignal is_engine_running_input = signals::BoolSignal(false, signals::ValidityStatus::INVALID);
    signals::BoolSignal clear_fault_request_input = signals::BoolSignal(false, signals::ValidityStatus::INVALID);

    engine_overheat_protection::EngineOverheatProtection eop_test (
        create_default_config,
        oil_temp_input,
        is_engine_running_input,
        clear_fault_request_input,
        manager
    );

    CycleInput scenario_input[] = {
        
        // {oil_temp_value, oil_temp_validity, is_engine_running_value, is_engine_running_validity, clear_fault_request_value, clear_fault_request_validity}
        {78.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // IDLE
        {78.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // FAULT
        
        {78.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // IDLE
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},   // FAULT

        {78.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // IDLE
        {78.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID}, // FAULT

        {78.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // IDLE
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP

        {78.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // IDLE
        {110.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // COUNTING

    };

    CycleOutput scenario_output[] = {

        // {is_overheat_protection_output, validity, torque_limit_output, validity, fan_request_output, validity, state, fault_reason}
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 30.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE},
        {false, signals::ValidityStatus::VALID, 80.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},

    };

    std::size_t size_of_input = sizeof(scenario_input) / sizeof(scenario_input[0]);
    std::size_t size_of_output = sizeof(scenario_output) / sizeof(scenario_output[0]);

    assert(size_of_output == size_of_input);

    for (std::size_t i = 0; i < size_of_input; ++i) {
        oil_temp_input.Set(scenario_input[i].oil_temp_value, scenario_input[i].oil_temp_validity);
        is_engine_running_input.Set(scenario_input[i].is_engine_running_value, scenario_input[i].is_engine_running_validity);

        manager.UpdateAll();

        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_output[i].is_overheat_protection_output);
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_output[i].is_overheat_protection_validity);
        // std::cout << eop_test.TorqueLimitRef().GetValue() << " " << scenario_output[i].torque_limit_output << std::endl;
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_output[i].torque_limit_output));
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_output[i].torque_limit_validity);
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_output[i].fan_request_output));
        assert(eop_test.FanRequestRef().GetValidity() == scenario_output[i].fan_request_validity); 

        assert(eop_test.StateRef() == scenario_output[i].state_);
    }
}

void CountingStateTest() {
    framework::Manager manager;

    struct CycleInput {

        float oil_temp_value;
        signals::ValidityStatus oil_temp_validity;
        bool is_engine_running_value;
        signals::ValidityStatus is_engine_running_validity;

    };

    struct CycleOutput {

        bool is_engine_overheat_protection_output;
        signals::ValidityStatus is_engine_overheat_protection_validity;
        float torque_limit_output;
        signals::ValidityStatus torque_limit_validity;
        float fan_request_output;
        signals::ValidityStatus fan_request_validity;

        engine_overheat_protection::EngineOverheatProtectionState state;

    };

    auto create_default_config = CreateDefaultConfig();

    signals::FloatSignal oil_temp_input = signals::FloatSignal(0.0f, signals::ValidityStatus::INVALID);
    signals::BoolSignal is_engine_running_input = signals::BoolSignal(false, signals::ValidityStatus::INVALID);
    signals::BoolSignal clear_fault_request_input = signals::BoolSignal(false, signals::ValidityStatus::VALID);

    engine_overheat_protection::EngineOverheatProtection eop_test (
        create_default_config,
        oil_temp_input,
        is_engine_running_input,
        clear_fault_request_input,
        manager
    );

    CycleInput scenario_input[] = {

        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {90.0f, signals::ValidityStatus::INVALID, true, signals::ValidityStatus::VALID},    // STOP

        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::INVALID},    // STOP

        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {90.0f, signals::ValidityStatus::INVALID, true, signals::ValidityStatus::INVALID},  // STOP

        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // IDLE

        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {98.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // AFTER_RUN_COOLING

        {91.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {93.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {95.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {97.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // PROTECTED

    };

    CycleOutput scenario_output[] = {

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 60.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 42.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 47.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 52.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 57.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED},

    };

    std::size_t size_of_input = sizeof(scenario_input) / sizeof(scenario_input[0]);
    std::size_t size_of_output = sizeof(scenario_output) / sizeof(scenario_output[0]);

    assert(size_of_output == size_of_input);

    for (std::size_t i = 0; i < size_of_input; ++i) {
        oil_temp_input.Set(scenario_input[i].oil_temp_value, scenario_input[i].oil_temp_validity);
        is_engine_running_input.Set(scenario_input[i].is_engine_running_value, scenario_input[i].is_engine_running_validity);

        manager.UpdateAll();

        // std::cout << eop_test.IsOverheatProtectedRef().GetValue() << " " << scenario_output[i].is_engine_overheat_protection_output << std::endl;
        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_output[i].is_engine_overheat_protection_output);
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_output[i].is_engine_overheat_protection_validity);
        // std::cout << "Torque_limit: " << eop_test.TorqueLimitRef().GetValue() << " " << scenario_output[i].torque_limit_output << std::endl;
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_output[i].torque_limit_output));
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_output[i].torque_limit_validity);
        // std::cout << "Fan_request: " << eop_test.FanRequestRef().GetValue() << " " << scenario_output[i].fan_request_output << std::endl;
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_output[i].fan_request_output));
        assert(eop_test.FanRequestRef().GetValidity() == scenario_output[i].fan_request_validity);

        assert(eop_test.StateRef() == scenario_output[i].state);
    }
}

void ProtectedStateTest() {
    framework::Manager manager;

    struct CycleInput {
        float oil_temp_value;
        signals::ValidityStatus oil_temp_validity;
        bool is_engine_running_value;
        signals::ValidityStatus is_engine_running_validity;
    };

    struct CycleOutput {
        bool is_engine_overheat_protection_output;
        signals::ValidityStatus is_engine_overheat_protection_validity;
        float torque_limit_output;
        signals::ValidityStatus torque_limit_validity;
        float fan_request_output;
        signals::ValidityStatus fan_request_validity;

        engine_overheat_protection::EngineOverheatProtectionState state;
    };

    signals::FloatSignal oil_temp_input = signals::FloatSignal(0.0f, signals::ValidityStatus::INVALID);
    signals::BoolSignal is_engine_running_value = signals::BoolSignal(false, signals::ValidityStatus::INVALID);
    signals::BoolSignal clear_fault_request_input = signals::BoolSignal(false, signals::ValidityStatus::VALID);

    auto create_default_config = CreateDefaultConfig();

    engine_overheat_protection::EngineOverheatProtection eop_test (
        create_default_config,
        oil_temp_input,
        is_engine_running_value,
        clear_fault_request_input,
        manager
    );

    CycleInput scenario_input[] = {

        {91.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {93.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {95.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {97.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // PROTECTED
        {97.0f, signals::ValidityStatus::INVALID, true, signals::ValidityStatus::VALID},    // STOP

        {91.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {93.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {95.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {97.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // PROTECTED
        {97.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::INVALID},    // STOP

        {91.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {93.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {95.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {97.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // PROTECTED
        {97.0f, signals::ValidityStatus::INVALID, true, signals::ValidityStatus::INVALID},  // STOP

        {91.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {93.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {95.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {97.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // PROTECTED
        {82.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // PROTECTED
        {75.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // IDLE

        {91.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {93.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {95.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // COUNTING
        {97.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // PROTECTED
        {80.01f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},    // AFTER_RUN_COOLING

    };

    CycleOutput scenario_output[] = {

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 42.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 47.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 52.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 57.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 42.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 47.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 52.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 57.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 42.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 47.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 52.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 57.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::STOP},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 42.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 47.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 52.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 57.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 24.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 42.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 47.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 52.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 57.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.02f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING},

    };

    std::size_t size_of_inputs = sizeof(scenario_input) / sizeof(scenario_input[0]);
    std::size_t size_of_outputs = sizeof(scenario_output) / sizeof(scenario_output[0]);

    assert(size_of_outputs == size_of_inputs);

    for (std::size_t i = 0; i < size_of_inputs; ++i) {
        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_output[i].is_engine_overheat_protection_output);
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_output[i].is_engine_overheat_protection_validity);
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_output[i].torque_limit_output));
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_output[i].torque_limit_validity);
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_output[i].fan_request_output));
        assert(eop_test.FanRequestRef().GetValidity() == scenario_output[i].fan_request_validity);

        assert(eop_test.StateRef() == scenario_output[i].state);
    }
}

void FaultStateTest() {
    framework::Manager manager;

    struct CycleInput {
        float oil_temp_value;
        signals::ValidityStatus oil_temp_validity;
        bool is_engine_running_value;
        signals::ValidityStatus is_engine_running_validity;
    };

    struct CycleOutput {
        bool is_engine_overheat_protection_output;
        signals::ValidityStatus is_engine_overheat_protection_validity;
        float torque_limit_output;
        signals::ValidityStatus torque_limit_validity;
        float fan_request_output;
        signals::ValidityStatus fan_request_validity;

        engine_overheat_protection::EngineOverheatProtectionState state;
    };

    signals::FloatSignal oil_temp_input = signals::FloatSignal(0.0f, signals::ValidityStatus::INVALID);
    signals::BoolSignal is_engine_running_value = signals::BoolSignal(false, signals::ValidityStatus::INVALID);
    signals::BoolSignal clear_fault_request_input = signals::BoolSignal(false, signals::ValidityStatus::VALID);

    auto create_default_config = CreateDefaultConfig();

    engine_overheat_protection::EngineOverheatProtection eop_test (
        create_default_config,
        oil_temp_input,
        is_engine_running_value,
        clear_fault_request_input,
        manager
    );

    CycleInput scenario_inputs[] = {
        {}
    };
}

int main() {
    std::cout << "============ Engine Overheat Protection Tests ============" << std::endl;
    std::cout << std::endl;
    const auto stop_state_start_time = std::chrono::steady_clock::now();
    std::cout << "============ Engine Overheat Protection Stop state Tests Starting ============" << std::endl;
    StopStateTest();
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

    // const auto idle_state_start_time = std::chrono::steady_clock::now();
    // std::cout << "============ Engine Overheat Protection Idle state Tests Starting ============" << std::endl;
    // IdleStateTest();
    // std::cout << "============ Engine Overheat Protection Idle state Tests Ending ============" << std::endl;
    // const auto idle_state_end_time = std::chrono::steady_clock::now();
    // const auto idle_state_elapsed_us = 
    //     std::chrono::duration_cast<std::chrono::microseconds> (
    //         idle_state_end_time - idle_state_start_time
    //     ).count();
    // std::cout << "Idle state test elapsed time: "
    //           << idle_state_elapsed_us
    //           << " us"
    //           << std::endl;
    // std::cout << std::endl;

    // const auto counting_state_start_time = std::chrono::steady_clock::now();
    // std::cout << "============ Engine Overheat Protection Counting state Tests Starting ============" << std::endl;
    // CountingStateTest();
    // std::cout << "============ Engine Overheat Protection Counting state Tests Ending ============" << std::endl;
    // const auto counting_state_end_time = std::chrono::steady_clock::now();
    // const auto counting_state_elapsed_us = 
    //     std::chrono::duration_cast<std::chrono::microseconds> (
    //         counting_state_end_time - counting_state_start_time
    //     ).count();
    // std::cout << "Counting state test elapsed time: "
    //           << counting_state_elapsed_us
    //           << " us"
    //           << std::endl;
    // std::cout << std::endl;

    // const auto protected_state_start_time = std::chrono::steady_clock::now();
    // std::cout << "============ Engine Overheat Protection Protected state Tests Starting ============" << std::endl;
    // ProtectedStateTest();
    // std::cout << "============ Engine Overheat Protection Protected state Tests Ending ============" << std::endl;
    // const auto protected_state_end_time = std::chrono::steady_clock::now();
    // const auto protected_state_elapsed_us = 
    //     std::chrono::duration_cast<std::chrono::microseconds> (
    //         protected_state_end_time - protected_state_start_time
    //     ).count();
    // std::cout << "Protected state test elapsed time: "
    //           << protected_state_elapsed_us 
    //           << " us"
    //           << std::endl;
    // std::cout << std::endl;

    return 0;
}
