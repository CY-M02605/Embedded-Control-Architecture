#include <iostream>
#include "framework/manager.h"
#include "signals/signal.h"
#include "vehicle_speed_control.h"

int main() {
    framework::Manager manager;

    // Input signals
    signals::FloatSignal engine_rpm(1500.0f, signals::ValidityStatus::VALID);
    signals::IntSignal gear_ratio(3, signals::ValidityStatus::VALID);

    // Create adapter module
    vehicle_speed_control::VehicleSpeedControl speed_module(engine_rpm, gear_ratio, manager);
    speed_module.Init();

    struct TestCase {
        float rpm;
        int gear;
    };

    TestCase tests[] = {
        {1500.0f, 3},
        {2000.0f, 4},
        {3000.0f, 5},
        {1000.0f, 1},
        { 800.0f, 0},  // gear=0, should output 0
    };

    int count = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < count; ++i) {
        engine_rpm.SetValue(tests[i].rpm);
        gear_ratio.SetValue(tests[i].gear);
        manager.UpdateAll();

        std::cout << "Cycle " << (i + 1)
                  << ": RPM=" << tests[i].rpm
                  << " Gear=" << tests[i].gear
                  << " Speed=" << speed_module.VehicleSpeedRef().GetValue()
                  << std::endl;
    }

    return 0;
}
