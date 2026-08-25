/**
 * @file assert_based_tests.cpp
 * @brief Provide a PC-based module test for engine overheat protection Arduino version
 * @date 2026-08-04 - 2026-08-25
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

const char* StateToStr(engine_overheat_protection::EngineOverheatProtectionState s) {
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

    config.oil_temp_low_threshold = 80.0f;
    config.oil_temp_high_threshold = 90.0f;
    config.oil_temp_physical_max = 125.0f;
    config.oil_temp_physical_min = -25.0f;
    config.oil_temp_low_fault_recover_critical_value = 15.0f;
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

        // {oil_temp_value, oil_temp_validity, is_engine_running_value, is_engine_running_validity, clear_fault_request_value, clear_fault_request_validity}
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP
        {78.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // FAULT: OIL_TEMP_SIGNAL_INVALID
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // STOP (recover)

        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},   // FAULT: ENGINE_RUNNING_SIGNAL_INVALID
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // STOP (recover)

        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP
        {125.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},    // FAULT: TEMP_OUT_OF_RANGE_HIGH
        {81.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // FAULT: TEMP_OUT_OF_RANGE_HIGH
        {80.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // STOP (recover)
        
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP
        {89.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP
        {90.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // FAULT: UNEXPECTED_HIGH_TEMP_IN_STOP
        {81.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // FAULT: UNEXPECTED_HIGH_TEMP_IN_STOP
        {80.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // STOP (recover)

        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP
        {-24.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},    // STOP
        {-25.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},    // FAULT: TEMP_OUT_OF_RANGE_LOW
        {14.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // FAULT: TEMP_OUT_OF_RANGE_LOW
        {15.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // STOP (recover)

        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP
        {78.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // IDLE

    };

    CycleOutput scenario_outputs[] = {
        
        // {is_overheat_protection_output, validity, torque_limit_output, validity, fan_request_output, validity, state, fault_reason}
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::OIL_TEMP_SIGNAL_INVALID}, 
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::ENGINE_RUNNING_SIGNAL_INVALID}, 
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH}, 
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH}, 
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::UNEXPECTED_HIGH_TEMP_IN_STOP},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::UNEXPECTED_HIGH_TEMP_IN_STOP}, 
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW}, 
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE}
        
    };

    std::size_t size_of_inputs = sizeof(scenario_inputs) / sizeof(scenario_inputs[0]);
    std::size_t size_of_outputs = sizeof(scenario_outputs) / sizeof(scenario_outputs[0]);

    assert(size_of_outputs == size_of_inputs);

    for (std::size_t i = 0; i < size_of_inputs; ++i) {
        oil_temp_input.Set(scenario_inputs[i].oil_temp_value, scenario_inputs[i].oil_temp_validity);
        is_engine_running_input.Set(scenario_inputs[i].is_engine_running_value, scenario_inputs[i].is_engine_running_validity);
        clear_fault_request_input.Set(scenario_inputs[i].clear_fault_request_value, scenario_inputs[i].clear_fault_request_validity);

        manager.UpdateAll();

        std::cout << "No." << i+1 << " | ";
        std::cout << eop_test.IsOverheatProtectedRef().GetValue()  << " " << scenario_outputs[i].is_overheat_protection_value << " | ";
        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_outputs[i].is_overheat_protection_value);
        std::cout << ValidityToStr(eop_test.IsOverheatProtectedRef().GetValidity())  << " " << ValidityToStr(scenario_outputs[i].is_overheat_protection_validity) << " | ";
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_outputs[i].is_overheat_protection_validity);
        std::cout << eop_test.TorqueLimitRef().GetValue() << " " << scenario_outputs[i].torque_limit_value << " | ";
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_outputs[i].torque_limit_value));
        std::cout << ValidityToStr(eop_test.TorqueLimitRef().GetValidity())  << " " << ValidityToStr(scenario_outputs[i].torque_limit_validity) << " | ";
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_outputs[i].torque_limit_validity);
        std::cout << eop_test.FanRequestRef().GetValue()  << " " << scenario_outputs[i].fan_request_value << " | ";
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_outputs[i].fan_request_value));
        std::cout << ValidityToStr(eop_test.FanRequestRef().GetValidity())  << " " << ValidityToStr(scenario_outputs[i].fan_request_validity) << " | ";
        assert(eop_test.FanRequestRef().GetValidity() == scenario_outputs[i].fan_request_validity); 

        std::cout << StateToStr(eop_test.StateRef())  << " " << StateToStr(scenario_outputs[i].state) << " | ";
        assert(eop_test.StateRef() == scenario_outputs[i].state);
        std::cout << FaultReasonToStr(eop_test.FaultReasonRef())  << " " << FaultReasonToStr(scenario_outputs[i].fault_reason) << std::endl;
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

        bool is_overheat_protection_value;
        signals::ValidityStatus is_overheat_protection_validity;
        float torque_limit_value;
        signals::ValidityStatus torque_limit_validity;
        float fan_request_value;
        signals::ValidityStatus fan_request_validity;

        engine_overheat_protection::EngineOverheatProtectionState state;
        engine_overheat_protection::FaultReason fault_reason;

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

    CycleInput scenario_inputs[] = {
        
        // {oil_temp_value, oil_temp_validity, is_engine_running_value, is_engine_running_validity, clear_fault_request_value, clear_fault_request_validity}
        {78.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // IDLE
        {78.0f, signals::ValidityStatus::INVALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},    // FAULT: OIL_TEMP_SIGNAL_INVALID
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // STOP (recover)

        {78.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},       // IDLE
        {78.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},    // FAULT: ENGINE_RUNNING_SIGNAL_INVALID
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // STOP (recover)

        {78.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // IDLE
        {125.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // FAULT: TEMP_OUT_OF_RANGE_HIGH
        {126.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // FAULT: TEMP_OUT_OF_RANGE_HIGH
        {81.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // FAULT: TEMP_OUT_OF_RANGE_HIGH
        {80.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // STOP (recover)

        {78.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // IDLE
        {-24.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // IDLE
        {-25.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // FAULT: TEMP_OUT_OF_RANGE_LOW
        {-26.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // FAULT: TEMP_OUT_OF_RANGE_LOW
        {14.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},       // FAULT: TEMP_OUT_OF_RANGE_LOW
        {15.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},      // STOP (recover)

        {78.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // IDLE
        {78.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // STOP

        {78.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // IDLE
        {89.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // IDLE
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // COUNTING
        {91.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // COUNTING
        {117.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // COUNTING

    };

    CycleOutput scenario_outputs[] = {

        // {is_overheat_protection_output, validity, torque_limit_output, validity, fan_request_output, validity, state, fault_reason}
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::OIL_TEMP_SIGNAL_INVALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::ENGINE_RUNNING_SIGNAL_INVALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 38.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 42.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 48.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},

    };

    std::size_t size_of_inputs = sizeof(scenario_inputs) / sizeof(scenario_inputs[0]);
    std::size_t size_of_outputs = sizeof(scenario_outputs) / sizeof(scenario_outputs[0]);

    assert(size_of_outputs == size_of_inputs);

    for (std::size_t i = 0; i < size_of_inputs; ++i) {
        oil_temp_input.Set(scenario_inputs[i].oil_temp_value, scenario_inputs[i].oil_temp_validity);
        is_engine_running_input.Set(scenario_inputs[i].is_engine_running_value, scenario_inputs[i].is_engine_running_validity);
        clear_fault_request_input.Set(scenario_inputs[i].clear_fault_request_value, scenario_inputs[i].clear_fault_request_validity);

        manager.UpdateAll();

        std::cout << "No." << i+1 << " | ";
        std::cout << eop_test.IsOverheatProtectedRef().GetValue()  << " " << scenario_outputs[i].is_overheat_protection_value << " | ";
        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_outputs[i].is_overheat_protection_value);
        std::cout << ValidityToStr(eop_test.IsOverheatProtectedRef().GetValidity())  << " " << ValidityToStr(scenario_outputs[i].is_overheat_protection_validity) << " | ";
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_outputs[i].is_overheat_protection_validity);
        std::cout << eop_test.TorqueLimitRef().GetValue() << " " << scenario_outputs[i].torque_limit_value << " | ";
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_outputs[i].torque_limit_value));
        std::cout << ValidityToStr(eop_test.TorqueLimitRef().GetValidity())  << " " << ValidityToStr(scenario_outputs[i].torque_limit_validity) << " | ";
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_outputs[i].torque_limit_validity);
        std::cout << eop_test.FanRequestRef().GetValue()  << " " << scenario_outputs[i].fan_request_value << " | ";
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_outputs[i].fan_request_value));
        std::cout << ValidityToStr(eop_test.FanRequestRef().GetValidity())  << " " << ValidityToStr(scenario_outputs[i].fan_request_validity) << " | ";
        assert(eop_test.FanRequestRef().GetValidity() == scenario_outputs[i].fan_request_validity); 

        std::cout << StateToStr(eop_test.StateRef())  << " " << StateToStr(scenario_outputs[i].state) << " | ";
        assert(eop_test.StateRef() == scenario_outputs[i].state);
        std::cout << FaultReasonToStr(eop_test.FaultReasonRef())  << " " << FaultReasonToStr(scenario_outputs[i].fault_reason) << std::endl;
        assert(eop_test.FaultReasonRef() == scenario_outputs[i].fault_reason);
    }
}

void CountingStateTest() {
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

        bool is_engine_overheat_protection_output;
        signals::ValidityStatus is_engine_overheat_protection_validity;
        float torque_limit_output;
        signals::ValidityStatus torque_limit_validity;
        float fan_request_output;
        signals::ValidityStatus fan_request_validity;

        engine_overheat_protection::EngineOverheatProtectionState state;
        engine_overheat_protection::FaultReason fault_reason;

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

    CycleInput scenario_inputs[] = {

        // {oil_temp_value, oil_temp_validity, is_engine_running_value, is_engine_running_validity, clear_fault_request_value, clear_fault_request_validity}
        {89.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // IDLE
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // COUNTING
        {90.0f, signals::ValidityStatus::INVALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID}, // FAULT: OIL_TEMP_SIGNAL_INVALID
        {80.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},   // STOP (recover)

        {89.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // IDLE
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // COUNTING
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID}, // FAULT: OIL_TEMP_SIGNAL_INVALID
        {80.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},   // STOP (recover)

        {89.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // IDLE
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // COUNTING
        {124.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},  // COUNTING
        {125.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},  // FAULT: TEMP_OUT_OF_RANGE_HIGH
        {126.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},  // FAULT: TEMP_OUT_OF_RANGE_HIGH
        {81.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},   // FAULT: TEMP_OUT_OF_RANGE_HIGH
        {80.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},   // STOP (recover)
        {79.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},   // STOP (recover)

        {89.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // IDLE
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // COUNTING
        {-25.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},  // FAULT: TEMP_OUT_OF_RANGE_LOW
        {-26.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},  // FAULT: TEMP_OUT_OF_RANGE_LOW
        {14.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},   // FAULT: TEMP_OUT_OF_RANGE_LOW
        {15.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},   // STOP (recover)
        {16.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},   // STOP (recover)

        {89.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // IDLE
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // COUNTING
        {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // IDLE

        {89.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // IDLE
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // COUNTING
        {98.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},  // AFTER_RUN_COOLING

        {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // STOP
        {89.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // IDLE
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // COUNTING
        {93.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // COUNTING
        {97.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // COUNTING
        {104.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},  // PROTECTED

    };

    CycleOutput scenario_outputs[] = {

        // {is_overheat_protection_output, validity, torque_limit_output, validity, fan_request_output, validity, state, fault_reason}
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 38.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::OIL_TEMP_SIGNAL_INVALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 38.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::ENGINE_RUNNING_SIGNAL_INVALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 38.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 30.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 38.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 38.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 38.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 60.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 38.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 47.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 57.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 96.0f, signals::ValidityStatus::VALID, 77.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},

    };

    std::size_t size_of_inputs = sizeof(scenario_inputs) / sizeof(scenario_inputs[0]);
    std::size_t size_of_outputs = sizeof(scenario_outputs) / sizeof(scenario_outputs[0]);

    assert(size_of_outputs == size_of_inputs);

    for (std::size_t i = 0; i < size_of_inputs; ++i) {
        oil_temp_input.Set(scenario_inputs[i].oil_temp_value, scenario_inputs[i].oil_temp_validity);
        is_engine_running_input.Set(scenario_inputs[i].is_engine_running_value, scenario_inputs[i].is_engine_running_validity);
        clear_fault_request_input.Set(scenario_inputs[i].clear_fault_request_value, scenario_inputs[i].clear_fault_request_validity);

        manager.UpdateAll();

        std::cout << "No." << i+1 << " | ";
        std::cout << eop_test.IsOverheatProtectedRef().GetValue() << " " << scenario_outputs[i].is_engine_overheat_protection_output << " | ";
        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_outputs[i].is_engine_overheat_protection_output);
        std::cout << ValidityToStr(eop_test.IsOverheatProtectedRef().GetValidity()) << " " << ValidityToStr(scenario_outputs[i].is_engine_overheat_protection_validity) << " | ";
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_outputs[i].is_engine_overheat_protection_validity);
        std::cout << eop_test.TorqueLimitRef().GetValue() << " " << scenario_outputs[i].torque_limit_output << " | ";
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_outputs[i].torque_limit_output));
        std::cout << ValidityToStr(eop_test.TorqueLimitRef().GetValidity()) << " " << ValidityToStr(scenario_outputs[i].torque_limit_validity) << " | ";
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_outputs[i].torque_limit_validity);
        std::cout << eop_test.FanRequestRef().GetValue() << " " << scenario_outputs[i].fan_request_output << " | ";
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_outputs[i].fan_request_output));
        std::cout << ValidityToStr(eop_test.FanRequestRef().GetValidity()) << " " << ValidityToStr(scenario_outputs[i].fan_request_validity) << " | ";
        assert(eop_test.FanRequestRef().GetValidity() == scenario_outputs[i].fan_request_validity);

        std::cout << StateToStr(eop_test.StateRef()) << " " << StateToStr(scenario_outputs[i].state) << " | ";
        assert(eop_test.StateRef() == scenario_outputs[i].state);
        std::cout << FaultReasonToStr(eop_test.FaultReasonRef()) << " " << FaultReasonToStr(scenario_outputs[i].fault_reason) << std::endl;
        assert(eop_test.FaultReasonRef() == scenario_outputs[i].fault_reason);
    }
}

void ProtectedStateTest() {
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

        bool is_engine_overheat_protection_output;
        signals::ValidityStatus is_engine_overheat_protection_validity;
        float torque_limit_output;
        signals::ValidityStatus torque_limit_validity;
        float fan_request_output;
        signals::ValidityStatus fan_request_validity;

        engine_overheat_protection::EngineOverheatProtectionState state;
        engine_overheat_protection::FaultReason fault_reason;

    };

    signals::FloatSignal oil_temp_input = signals::FloatSignal(0.0f, signals::ValidityStatus::INVALID);
    signals::BoolSignal is_engine_running_input = signals::BoolSignal(false, signals::ValidityStatus::INVALID);
    signals::BoolSignal clear_fault_request_input = signals::BoolSignal(false, signals::ValidityStatus::VALID);

    auto create_default_config = CreateDefaultConfig();

    engine_overheat_protection::EngineOverheatProtection eop_test (
        create_default_config,
        oil_temp_input,
        is_engine_running_input,
        clear_fault_request_input,
        manager
    );

    CycleInput scenario_inputs[] = {

        // {oil_temp_value, oil_temp_validity, is_engine_running_value, is_engine_running_validity, clear_fault_request_value, clear_fault_request_validity}
        {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 1:IDLE
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 2:COUNTING
        {94.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 3:COUNTING
        {98.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 4:COUNTING
        {102.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 5:PROTECTED
        {102.0f, signals::ValidityStatus::INVALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},    // 6:FAULT: OIL_TEMP_SIGNAL_INVALID
        {81.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},       // 7:FAULT: OIL_TEMP_SIGNAL_INVALID
        {80.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},       // 8:STOP (recover)

        {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 9:IDLE
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 10:COUNTING
        {92.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 11:COUNTING
        {95.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 12:COUNTING
        {101.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 13:PROTECTED
        {101.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},    // 14:FAULT: ENGINE_RUNNING_SIGNAL_INVALID
        {81.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},     // 15:FAULT: ENGINE_RUNNING_SIGNAL_INVALID
        {80.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},       // 16:STOP (recover)

        {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 17:IDLE
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 18:COUNTING
        {92.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 19:COUNTING
        {95.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 20:COUNTING
        {101.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 21:PROTECTED
        {124.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 22:PROTECTED
        {125.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 23:FAULT: TEMP_OUT_OF_RANGE_HIGH
        {81.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 24:FAULT: TEMP_OUT_OF_RANGE_HIGH
        {80.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},       // 25:STOP (recover)

        {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 23:IDLE
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 24:COUNTING
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 25:COUNTING
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 26:COUNTING
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 27:PROTECTED
        {-25.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 28:FAULT: TEMP_OUT_OF_RANGE_LOW
        {14.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},       // 29:FAULT: TEMP_OUT_OF_RANGE_LOW
        {15.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},       // 30:STOP (recover)

        {87.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 31:IDLE
        {94.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 32:COUNTING
        {101.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 33:COUNTING
        {109.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 34:COUNTING
        {113.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 35:PROTECTED
        {81.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 36:PROTECTED
        {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 37:IDLE

        {87.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 38:IDLE
        {94.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 39:COUNTING
        {101.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 40:COUNTING
        {109.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 41:COUNTING
        {113.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 42:PROTECTED
        {113.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID}      // 43:AFTER_RUN_COOLING

    };

    CycleOutput scenario_outputs[] = {

        // {is_overheat_protection_output, validity, torque_limit_output, validity, fan_request_output, validity, state, fault_reason}
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 50.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 60.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 98.0f, signals::ValidityStatus::VALID, 71.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::OIL_TEMP_SIGNAL_INVALID},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::OIL_TEMP_SIGNAL_INVALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 45.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 52.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 99.0f, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::ENGINE_RUNNING_SIGNAL_INVALID},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::ENGINE_RUNNING_SIGNAL_INVALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 45.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 52.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 99.0f, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 30.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 34.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 50.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 99.0f, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 83.0f, signals::ValidityStatus::VALID, 96.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 22.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 20.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 34.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 50.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 99.0f, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 83.0f, signals::ValidityStatus::VALID, 96.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING, engine_overheat_protection::FaultReason::NONE}

    };

    std::size_t size_of_inputs = sizeof(scenario_inputs) / sizeof(scenario_inputs[0]);
    std::size_t size_of_outputs = sizeof(scenario_outputs) / sizeof(scenario_outputs[0]);

    assert(size_of_outputs == size_of_inputs);

    for (std::size_t i = 0; i < size_of_inputs; ++i) {
        oil_temp_input.Set(scenario_inputs[i].oil_temp_value, scenario_inputs[i].oil_temp_validity);
        is_engine_running_input.Set(scenario_inputs[i].is_engine_running_value, scenario_inputs[i].is_engine_running_validity);
        clear_fault_request_input.Set(scenario_inputs[i].clear_fault_request_value, scenario_inputs[i].clear_fault_request_validity);

        manager.UpdateAll();

        std::cout << "No." << i+1 << " | ";
        std::cout << eop_test.IsOverheatProtectedRef().GetValue() << " " << scenario_outputs[i].is_engine_overheat_protection_output << " | ";
        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_outputs[i].is_engine_overheat_protection_output);
        std::cout << ValidityToStr(eop_test.IsOverheatProtectedRef().GetValidity()) << " " << ValidityToStr(scenario_outputs[i].is_engine_overheat_protection_validity) << " | ";
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_outputs[i].is_engine_overheat_protection_validity);
        std::cout << eop_test.TorqueLimitRef().GetValue() << " " << scenario_outputs[i].torque_limit_output << " | ";
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_outputs[i].torque_limit_output));
        std::cout << ValidityToStr(eop_test.TorqueLimitRef().GetValidity()) << " " << ValidityToStr(scenario_outputs[i].torque_limit_validity) << " | ";
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_outputs[i].torque_limit_validity);
        std::cout << eop_test.FanRequestRef().GetValue() << " " << scenario_outputs[i].fan_request_output << " | ";
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_outputs[i].fan_request_output));
        std::cout << ValidityToStr(eop_test.FanRequestRef().GetValidity()) << " " << ValidityToStr(scenario_outputs[i].fan_request_validity) << " | ";
        assert(eop_test.FanRequestRef().GetValidity() == scenario_outputs[i].fan_request_validity);

        std::cout << StateToStr(eop_test.StateRef()) << " " << StateToStr(scenario_outputs[i].state) << " | ";
        assert(eop_test.StateRef() == scenario_outputs[i].state);
        std::cout << FaultReasonToStr(eop_test.FaultReasonRef()) << " " << FaultReasonToStr(scenario_outputs[i].fault_reason) << std::endl;
        assert(eop_test.FaultReasonRef() == scenario_outputs[i].fault_reason);
    }
}

void AfterRunCoolingStateTest() {
    framework::Manager manager;

    struct CycleInput {

        float oil_temp_value;
        signals::ValidityStatus oil_temp_validity;
        bool is_engine_running_value;
        signals::ValidityStatus is_engine_running_validity;
        bool fault_clear_request_value;
        signals::ValidityStatus fault_clear_request_validity;

    };

    struct CycleOutput {

        bool is_engine_overheat_protection_output;
        signals::ValidityStatus is_engine_overheat_protection_validity;
        float torque_limit_output;
        signals::ValidityStatus torque_limit_validity;
        float fan_request_output;
        signals::ValidityStatus fan_request_validity;

        engine_overheat_protection::EngineOverheatProtectionState state;
        engine_overheat_protection::FaultReason fault_reason;

    };

    auto create_default_config = CreateDefaultConfig();

    signals::FloatSignal oil_temp_input = signals::FloatSignal(0.0f, signals::ValidityStatus::INVALID); 
    signals::BoolSignal is_engine_running_input = signals::BoolSignal(false, signals::ValidityStatus::INVALID);
    signals::BoolSignal fault_clear_request_input = signals::BoolSignal(false, signals::ValidityStatus::VALID);

    engine_overheat_protection::EngineOverheatProtection eop_test (
        create_default_config,
        oil_temp_input,
        is_engine_running_input,
        fault_clear_request_input,
        manager
    );

    CycleInput scenario_inputs[] = {

        // {oil_temp_value, oil_temp_validity, is_engine_running_value, is_engine_running_validity, clear_fault_request_value, clear_fault_request_validity}
        {87.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 1:IDLE
        {94.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 2:COUNTING
        {101.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 3:COUNTING
        {109.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 4:COUNTING
        {113.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 5:PROTECTED
        {113.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // 6:AFTER_RUN_COOLING
        {113.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},   // 7:FAULT: OIL_TEMP_SIGNAL_INVALID
        {80.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},       // 8:STOP

        {87.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 9:IDLE
        {94.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 10:COUNTING
        {101.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 11:COUNTING
        {109.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 12:COUNTING
        {113.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 13:PROTECTED
        {113.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // 14:AFTER_RUN_COOLING
        {113.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},   // 15:FAULT: ENGINE_RUNNING_SIGNAL_INVALID
        {80.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},       // 16:STOP

        {87.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 17:IDLE
        {94.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 18:COUNTING
        {101.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 19:COUNTING
        {109.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 20:COUNTING
        {113.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 21:PROTECTED
        {113.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // 22:AFTER_RUN_COOLING
        {124.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // 23:AFTER_RUN_COOLING
        {125.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // 24:FAULT: TEMP_OUT_OF_RANGE_HIGH
        {81.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 25:FAULT: TEMP_OUT_OF_RANGE_HIGH
        {80.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},       // 26:STOP

        {87.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 27:IDLE
        {94.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 28:COUNTING
        {101.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 29:COUNTING
        {109.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 30:COUNTING
        {113.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 31:PROTECTED
        {113.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // 32:AFTER_RUN_COOLING
        {-25.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // 33:FAULT: TEMP_OUT_OF_RANGE_LOW
        {14.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 34:FAULT: TEMP_OUT_OF_RANGE_LOW
        {15.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},       // 35:STOP

        {87.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 36:IDLE
        {94.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},       // 37:COUNTING
        {101.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 38:COUNTING
        {109.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 39:COUNTING
        {113.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 40:PROTECTED
        {113.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},     // 41:AFTER_RUN_COOLING
        {113.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 42:COUNTING

        {109.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 43:COUNTING
        {105.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 44:COUNTING
        {100.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 45:PROTECTED
        {95.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 46:AFTER_RUN_COOLING
        {90.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 47:AFTER_RUN_COOLING
        {85.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 48:AFTER_RUN_COOLING
        {81.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 49:AFTER_RUN_COOLING
        {80.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},      // 50:STOP
    };

    CycleOutput scenario_outputs[] = {

        // {is_overheat_protection_output, validity, torque_limit_output, validity, fan_request_output, validity, state, fault_reason}
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 34.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 50.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 99.0f, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 83.0f, signals::ValidityStatus::VALID, 96.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::OIL_TEMP_SIGNAL_INVALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 34.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 50.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 99.0f, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 83.0f, signals::ValidityStatus::VALID, 96.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::ENGINE_RUNNING_SIGNAL_INVALID},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 34.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 50.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 99.0f, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 83.0f, signals::ValidityStatus::VALID, 96.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 34.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 50.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 99.0f, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 83.0f, signals::ValidityStatus::VALID, 96.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW},
        {false, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, 0.0f, signals::ValidityStatus::INVALID, engine_overheat_protection::EngineOverheatProtectionState::FAULT, engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 34.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::IDLE, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 50.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 99.0f, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 83.0f, signals::ValidityStatus::VALID, 96.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 68.0f, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},

        {false, signals::ValidityStatus::VALID, 83.0f, signals::ValidityStatus::VALID, 96.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 95.0f, signals::ValidityStatus::VALID, 80.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::COUNTING, engine_overheat_protection::FaultReason::NONE},
        {true, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 65.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::PROTECTED, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 52.5f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 40.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 30.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 22.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING, engine_overheat_protection::FaultReason::NONE},
        {false, signals::ValidityStatus::VALID, 100.0f, signals::ValidityStatus::VALID, 0.0f, signals::ValidityStatus::VALID, engine_overheat_protection::EngineOverheatProtectionState::STOP, engine_overheat_protection::FaultReason::NONE},
    };

    std::size_t size_of_inputs = sizeof(scenario_inputs) / sizeof(scenario_inputs[0]);
    std::size_t size_of_outputs = sizeof(scenario_outputs) / sizeof(scenario_outputs[0]);

    assert(size_of_outputs == size_of_inputs);

    for (std::size_t i = 0; i < size_of_inputs; ++i) {
        oil_temp_input.Set(scenario_inputs[i].oil_temp_value, scenario_inputs[i].oil_temp_validity);
        is_engine_running_input.Set(scenario_inputs[i].is_engine_running_value, scenario_inputs[i].is_engine_running_validity);
        fault_clear_request_input.Set(scenario_inputs[i].fault_clear_request_value, scenario_inputs[i].fault_clear_request_validity);

        manager.UpdateAll();

        std::cout << "No." << i+1 << " | ";
        std::cout << eop_test.IsOverheatProtectedRef().GetValue() << " " << scenario_outputs[i].is_engine_overheat_protection_output << " | ";
        assert(eop_test.IsOverheatProtectedRef().GetValue() == scenario_outputs[i].is_engine_overheat_protection_output);
        std::cout << ValidityToStr(eop_test.IsOverheatProtectedRef().GetValidity()) << " " << ValidityToStr(scenario_outputs[i].is_engine_overheat_protection_validity) << " | ";
        assert(eop_test.IsOverheatProtectedRef().GetValidity() == scenario_outputs[i].is_engine_overheat_protection_validity);
        std::cout << eop_test.TorqueLimitRef().GetValue() << " " << scenario_outputs[i].torque_limit_output << " | ";
        assert(FloatEqual(eop_test.TorqueLimitRef().GetValue(), scenario_outputs[i].torque_limit_output));
        std::cout << ValidityToStr(eop_test.TorqueLimitRef().GetValidity()) << " " << ValidityToStr(scenario_outputs[i].torque_limit_validity) << " | ";
        assert(eop_test.TorqueLimitRef().GetValidity() == scenario_outputs[i].torque_limit_validity);
        std::cout << eop_test.FanRequestRef().GetValue() << " " << scenario_outputs[i].fan_request_output << " | ";
        assert(FloatEqual(eop_test.FanRequestRef().GetValue(), scenario_outputs[i].fan_request_output));
        std::cout << ValidityToStr(eop_test.FanRequestRef().GetValidity()) << " " << ValidityToStr(scenario_outputs[i].fan_request_validity) << " | ";
        assert(eop_test.FanRequestRef().GetValidity() == scenario_outputs[i].fan_request_validity);

        std::cout << StateToStr(eop_test.StateRef()) << " " << StateToStr(scenario_outputs[i].state) << " | ";
        assert(eop_test.StateRef() == scenario_outputs[i].state);
        std::cout << FaultReasonToStr(eop_test.FaultReasonRef()) << " " << FaultReasonToStr(scenario_outputs[i].fault_reason) << std::endl;
        assert(eop_test.FaultReasonRef() == scenario_outputs[i].fault_reason);
    }

}

int main() {

    std::cout << "============ Engine Overheat Protection Tests ============" << std::endl;
    std::cout << std::endl;

    // STOP state tests
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

    // IDLE state tests
    const auto idle_state_start_time = std::chrono::steady_clock::now();
    std::cout << "============ Engine Overheat Protection Idle state Tests Starting ============" << std::endl;
    IdleStateTest();
    std::cout << "============ Engine Overheat Protection Idle state Tests Ending ============" << std::endl;
    const auto idle_state_end_time = std::chrono::steady_clock::now();
    const auto idle_state_elapsed_us = 
        std::chrono::duration_cast<std::chrono::microseconds> (
            idle_state_end_time - idle_state_start_time
        ).count();
    std::cout << "Idle state test elapsed time: "
              << idle_state_elapsed_us
              << " us"
              << std::endl;
    std::cout << std::endl;

    // COUNTING state tests
    const auto counting_state_start_time = std::chrono::steady_clock::now();
    std::cout << "============ Engine Overheat Protection Counting state Tests Starting ============" << std::endl;
    CountingStateTest();
    std::cout << "============ Engine Overheat Protection Counting state Tests Ending ============" << std::endl;
    const auto counting_state_end_time = std::chrono::steady_clock::now();
    const auto counting_state_elapsed_us = 
        std::chrono::duration_cast<std::chrono::microseconds> (
            counting_state_end_time - counting_state_start_time
        ).count();
    std::cout << "Counting state test elapsed time: "
              << counting_state_elapsed_us
              << " us"
              << std::endl;
    std::cout << std::endl;

    // PROTECTED state tests
    const auto protected_state_start_time = std::chrono::steady_clock::now();
    std::cout << "============ Engine Overheat Protection Protected state Tests Starting ============" << std::endl;
    ProtectedStateTest();
    std::cout << "============ Engine Overheat Protection Protected state Tests Ending ============" << std::endl;
    const auto protected_state_end_time = std::chrono::steady_clock::now();
    const auto protected_state_elapsed_us = 
        std::chrono::duration_cast<std::chrono::microseconds> (
            protected_state_end_time - protected_state_start_time
        ).count();
    std::cout << "Protected state test elapsed time: "
              << protected_state_elapsed_us 
              << " us"
              << std::endl;
    std::cout << std::endl;

    // AFTER_RUN_COOLING state tests
    const auto after_run_cooling_state_start_time = std::chrono::steady_clock::now();
    std::cout << "============ Engine Overheat Protection After_run_cooling state Tests Starting ============" << std::endl;
    AfterRunCoolingStateTest();
    std::cout << "============ Engine Overheat Protection After_run_cooling state Tests Ending ============" << std::endl;
    const auto after_run_cooling_state_end_time = std::chrono::steady_clock::now();
    const auto after_run_cooling_state_elapsed_us = 
        std::chrono::duration_cast<std::chrono::microseconds> (
            after_run_cooling_state_end_time - after_run_cooling_state_start_time
        ).count();
    std::cout << "After_run_cooling state test elapsed time: "
              << after_run_cooling_state_elapsed_us 
              << " us"
              << std::endl;
    std::cout << std::endl;

    return 0;
}
