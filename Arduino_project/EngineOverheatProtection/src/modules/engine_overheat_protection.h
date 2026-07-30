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
#include "../utility/hysteresis.h"
#include "../utility/increment_timer.h"
#include "../utility/lookup_table_1d.h"

#include <cstddef>

namespace engine_overheat_protection {

enum class EngineOverheatProtectionState {
    IDLE,
    COUNTING,
    PROTECTED,
    AFTER_RUN_COOLING
};

class EngineOverheatProtection: public framework::ModuleInterface {
public:
    struct Config {
        float oil_high_threshold;
        float oil_low_threshold;
        utility::IncrementTimer::Config increment_timer_config;
        const utility::LookupTable1D<float>::Points* torque_lookup_table_points;
        const std::size_t torque_lookup_table_size;
        const utility::LookupTable1D<float>::Points* fan_request_lookup_table_points;
        const std::size_t fan_request_lookup_table_size;
    };

    explicit EngineOverheatProtection(
        const Config& config,
        const signals::FloatSignal& oil_temp,
        const signals::BoolSignal& is_engine_running,
        framework::Manager& manager
    );

    void Update() override;

    const signals::BoolSignal& IsOverheatProtectedRef() const;

    const signals::FloatSignal& TorqueLimitRef() const;

    const signals::FloatSignal& FanRequestRef() const;

private:
    const Config config_;
    const signals::FloatSignal& oil_temp_;
    const signals::BoolSignal& is_engine_running_;

    engine_overheat_protection::EngineOverheatProtectionState state_;

    signals::BoolSignal is_overheat_protection_output_;
    signals::FloatSignal torque_limit_output_;
    signals::FloatSignal fan_request_output_;

    utility::IncrementTimer increment_timer_;
    utility::LookupTable1D<float> torque_lookup_table_;
    utility::LookupTable1D<float> fan_request_lookup_table_;

};
}

#endif
