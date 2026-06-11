#include <iostream>

#include "signal.h"
#include "manager.h"

// #include "instantiation.h"
// #include "instantiation_practice.h"

// ============================================================
// Step 1: include instantiation.h to get extern declarations
//         (INSTANTIATION_DEFINE is NOT defined yet)
// ============================================================
#include "instantiation.h"
#include "instantiation_practice.h"   // â†? expands all INSTANTIATION() as "extern Type name;"

// ============================================================
// Step 2: define the shared input signals
//         (these are the "wires" that connect modules together)
// ============================================================
signals::FloatSignal        oil_temp(80.0f, signals::ValidityStatus::VALID);
signals::FloatSignal        vehicle_speed(0.0f, signals::ValidityStatus::VALID);
signals::Signal<float>      speed_limit(30.0f, signals::ValidityStatus::VALID);
framework::Manager          app_manager;

// ============================================================
// Step 3: define INSTANTIATION_DEFINE, then include again
//         Now all INSTANTIATION() calls expand to constructors
// ============================================================
#define INSTANTIATION_DEFINE
#include "instantiation.h"
#include "instantiation_practice.h"   // â†? expands all INSTANTIATION() as "Type name(args...);"
#undef INSTANTIATION_DEFINE

// ============================================================
// main: use the modules
// ============================================================
int main() {
    std::cout << "=== Instantiation Practice ===" << std::endl;

    // Test 1: normal temperature, normal speed
    oil_temp.SetValue(80.0f);
    vehicle_speed.SetValue(20.0f);
    app_manager.UpdateAll();

    std::cout << "[Test 1] temp=80, speed=20/30 => "
              << "warn=" << oil_temp_warning.WarnSignalRef().GetValue()
              << " (expect 0=false), "
              << "over=" << speed_monitor.OverSpeedRef().GetValue()
              << " (expect 0=false)"
              << std::endl;

    // Test 2: high temperature
    oil_temp.SetValue(98.0f);
    app_manager.UpdateAll();

    std::cout << "[Test 2] temp=98 => "
              << "warn=" << oil_temp_warning.WarnSignalRef().GetValue()
              << " (expect 1=true)"
              << std::endl;

    // Test 3: over speed
    oil_temp.SetValue(80.0f);
    vehicle_speed.SetValue(45.0f);
    app_manager.UpdateAll();

    std::cout << "[Test 3] speed=45, limit=30 => "
              << "over=" << speed_monitor.OverSpeedRef().GetValue()
              << " (expect 1=true)"
              << std::endl;

    std::cout << "=== Done ===" << std::endl;
    return 0;
}
