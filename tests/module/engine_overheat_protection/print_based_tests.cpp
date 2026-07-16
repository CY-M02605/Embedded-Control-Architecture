/**
 * @file main.cpp (for engine_overheat_protection.cpp)
 * @brief Provides a PC-based module test for the EngineOverheatProtection module.
 * @date 2026-07-03
 */

#include "manager.h"
#include "module_interface.h"

#include "signal.h"

#include "engine_overheat_protection.h"

#include <iostream>
#include <cassert>

const char* ValidityStr(signals::ValidityStatus s) {
    return s == signals::ValidityStatus::VALID? "VALID" : "INVALID";
}

int main() {
    std::cout << "============ Engine Overheat Protection Test ============" << std::endl;

    engine_overheat_protection::EngineOverheatProtection::Config config;

    const utility::LookupTable1D<float>::Point oil_temp_fan_request_table[] = {
        {80.0f, 20.0f},
        {90.0f, 40.0f},
        {100.0f, 65.0f},
        {105.0f, 80.0f},
        {110.0f, 100.0f},
        {115.0f, 100.0f},
        {120.0f, 100.0f}
    };

    const utility::LookupTable1D<float>::Point oil_temp_torque_limit_table[] = {
        {80.0f, 100.0f},
        {90.0f, 100.0f},
        {100.0f, 100.0f},
        {105.0f, 95.0f},
        {110.0f, 80.0f},
        {115.0f, 60.0f},
        {120.0f, 30.0f}
    };

    config.oil_low_threshold = 80;
    config.oil_high_threshold = 90;
    config.increment_timer_config.threshold_timer = 3;
    config.fan_lookup_table_points = oil_temp_fan_request_table;
    config.torque_lookup_table_points = oil_temp_torque_limit_table;
    config.fan_lookup_table_size = sizeof(oil_temp_fan_request_table) / sizeof(oil_temp_fan_request_table[0]);
    config.torque_lookup_table_size = sizeof(oil_temp_torque_limit_table) / sizeof(oil_temp_torque_limit_table[0]);

    framework::Manager manager;

    signals::FloatSignal oil_temp(0.0f, signals::ValidityStatus::VALID);
    signals::BoolSignal is_engine_running(false, signals::ValidityStatus::VALID);

    engine_overheat_protection::EngineOverheatProtection eop_test (
        config,
        oil_temp,
        is_engine_running,
        manager
    );

    struct CycleInput{
        float oil_temp;
        signals::ValidityStatus oil_temp_validity;
        bool is_engine_running;
        signals::ValidityStatus is_engine_running_validity;
    };

    CycleInput scenario[] = {
        {0.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},
        {80.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},  
        {90.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},  
        {100.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},  
        {105.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},  
        {110.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},  
        {120.0f, signals::ValidityStatus::INVALID, false, signals::ValidityStatus::VALID},  

        {0.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID},
        {80.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID},  
        {90.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID},  
        {100.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID},  
        {105.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID},  
        {110.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID},  
        {120.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::INVALID}, 

        {0.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},  
        {10.2f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {18.5f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {24.3f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {39.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {46.8f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {55.4f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {62.7f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {77.8f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {79.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {80.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {88.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {89.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {90.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {93.6f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {99.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {100.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {100.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {104.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {105.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {105.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {109.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {110.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {110.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {113.7f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {117.3f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {119.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {120.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {119.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {118.6f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID}, 
        {115.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {115.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {114.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {112.5f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},
        {110.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {110.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {109.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {107.2f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID}, 
        {105.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {105.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {104.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {100.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {100.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {99.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {94.9f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID}, 
        {90.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {90.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {89.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {87.3f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID}, 
        {80.1f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {80.0f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {79.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {78.5f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {64.1f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID},  
        {55.7f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {46.9f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {38.2f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {26.3f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},
        {17.4f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID},  
        {7.8f, signals::ValidityStatus::VALID, true, signals::ValidityStatus::VALID}, 
        {0.0f, signals::ValidityStatus::VALID, false, signals::ValidityStatus::VALID}
    };

    const std::size_t num_cycles = sizeof(scenario) / sizeof(scenario[0]);
    
    for (std::size_t i = 0; i < num_cycles; ++i) {
        oil_temp = signals::FloatSignal(
            scenario[i].oil_temp,
            scenario[i].oil_temp_validity
        );

        is_engine_running = signals::BoolSignal(
            scenario[i].is_engine_running,
            scenario[i].is_engine_running_validity
        );

        manager.UpdateAll();

        std::cout << "cycle: " << i + 1
                  << " | oil_temp_value: " << oil_temp.GetValue()
                  << " | oil_temp_validity: " << ValidityStr(oil_temp.GetValidity())
                  << " | engine_running_value: " << is_engine_running.GetValue()
                  << " | engine_running_validity: " << ValidityStr(is_engine_running.GetValidity())
                  << "\n"
                  << " | protected_value: "
                  << eop_test.IsOverheatProtectedRef().GetValue()
                  << " | protected_validity: "
                  << ValidityStr(eop_test.IsOverheatProtectedRef().GetValidity())
                  << "\n"
                  << " | fan_request_value: "
                  << eop_test.FanRequestRef().GetValue()
                  << " | fan_request_validity: "
                  << ValidityStr(eop_test.FanRequestRef().GetValidity())
                  << "\n"
                  << " | torque_limit_value: "
                  << eop_test.TorqueLimitRef().GetValue()
                  << " | torque_limit_value_validity: "
                  << ValidityStr(eop_test.TorqueLimitRef().GetValidity())
                  << std::endl;
    }
    return 0;
}
