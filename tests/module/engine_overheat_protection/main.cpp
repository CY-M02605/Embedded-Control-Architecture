/**
 * @file main.cpp (for engine_overheat_protection.cpp)
 * @brief module test for engine_overheat_protection module
 * @date 2026-07-03
 */

#include "manager.h"
#include "module_interface.h"

#include "signal.h"

#include "engine_overheat_protection.h"

#include <iostream>

int main() {
    std::cout << "============ Engine Overheat Protection Test ============" << std::endl;

    engine_overheat_protection::EngineOverheatProtection::Config config;

    utility::LookupTable1D<float>::Point OilTempFanRequestTable[] = {
        {80.0f, 20.0f},
        {90.0f, 40.0f},
        {100.0f, 65.0f},
        {105.0f, 80.0f},
        {110.0f, 100.0f},
        {115.0f, 100.0f},
        {120.0f, 100.0f}
    };

    utility::LookupTable1D<float>::Point OilTempTorqueLimitTable[] = {
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
    config.fan_lookup_table_points = OilTempFanRequestTable;
    config.torque_lookup_table_points = OilTempTorqueLimitTable;
    config.fan_lookup_table_size = sizeof(OilTempFanRequestTable) / sizeof(OilTempFanRequestTable[0]);
    config.torque_lookup_table_size = sizeof(OilTempTorqueLimitTable) / sizeof(OilTempTorqueLimitTable[0]);

    framework::Manager manager;

    signals::FloatSignal oil_temp(0.0f, signals::ValidityStatus::INVALID);
    signals::BoolSignal is_engine_running(false, signals::ValidityStatus::INVALID);

    engine_overheat_protection::EngineOverheatProtection EOP_Test (
        config,
        oil_temp,
        is_engine_running,
        manager
    );
    return 0;
}