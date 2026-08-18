/**
 * @file engine_overheat_protection.h
 * @brief create a header for Arduino project as an include file
 * @date 2026-07-29
 */

#ifndef ENGINE_OVERHEAT_PROTECTION_H
#define ENGINE_OVERHEAT_PROTECTION_H

#include "../framework/manager.h"
#include "../framework/module_interface.h"
#include "../signals/signal.h"
// #include "../utility/hysteresis.h"
#include "../utility/increment_timer.h"
#include "../utility/lookup_table_1d.h"

#include <cstddef>

namespace engine_overheat_protection {

enum class EngineOverheatProtectionState {
    STOP,
    IDLE,
    COUNTING,
    PROTECTED,
    AFTER_RUN_COOLING,
    FAULT
};

enum class FaultReason {
    NONE,
    OIL_TEMP_SIGNAL_INVALID,
    ENGINE_RUNNING_SIGNAL_INVALID,
    UNEXPECTED_HIGH_TEMP_IN_STOP,
    TEMP_RISE_TOO_FAST,
    TEMP_OUT_OF_RANGE_LOW,
    TEMP_OUT_OF_RANGE_HIGH,
    LOOKUP_TABLE_ERROR
};

class EngineOverheatProtection: public framework::ModuleInterface {
public:
    struct Config {
        float oil_high_threshold;
        float oil_low_threshold;
        utility::IncrementTimer::Config increment_timer_config;
        const utility::LookupTable1D<float>::Points* torque_lookup_table_points;
        std::size_t torque_lookup_table_size;
        const utility::LookupTable1D<float>::Points* fan_request_lookup_table_points;
        std::size_t fan_request_lookup_table_size;
    };

    explicit EngineOverheatProtection(
        const Config& config,
        const signals::FloatSignal& oil_temp,
        const signals::BoolSignal& is_engine_running,
        const signals::BoolSignal& clear_fault_request,
        framework::Manager& manager
    );

    void Update() override;

    const signals::BoolSignal& IsOverheatProtectedRef() const;

    const signals::FloatSignal& TorqueLimitRef() const;

    const signals::FloatSignal& FanRequestRef() const;

    const engine_overheat_protection::EngineOverheatProtectionState StateRef() const;

private:
    const Config config_;
    const signals::FloatSignal& oil_temp_;
    const signals::BoolSignal& is_engine_running_;

    const signals::BoolSignal& clear_fault_request_;

    engine_overheat_protection::EngineOverheatProtectionState state_;
    engine_overheat_protection::FaultReason fault_reason_;

    signals::BoolSignal is_overheat_protection_output_;
    signals::FloatSignal torque_limit_output_;
    signals::FloatSignal fan_request_output_;

    utility::IncrementTimer increment_timer_;
    utility::LookupTable1D<float> torque_lookup_table_;
    utility::LookupTable1D<float> fan_request_lookup_table_;

};
}

#endif
