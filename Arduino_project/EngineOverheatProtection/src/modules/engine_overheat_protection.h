/**
 * @file engine_overheat_protection.h
 * @brief create a header for Arduino project as an include file
 * @date 2026-07-29
 */

#include "src/framework/manager.h"
#include "src/framework/module_interface.h"
#include "src/signals/signal.h"
#include "src/utility/hysteresis.h"
#include "src/utility/increment_timer.h"
#include "src/utility/lookup_table_1d.h"

#include <cstddef>

#ifndef ENGINE_OVERHEAT_PROTECTION_H
#define ENGINE_OVERHEAT_PROTECTION_H

namespace engine_overheat_protection {

enum class EngineOverheatProtectionState {
    IDLE,
    COUNTING,
    PROTECTED,
    AFTER_RUN_COOLING
};

class EngineOverheatProtection: public module_interafce::ModuleInterface {
public:
    struct Config {
        float oil_high_threshold;
        float oil_low_threshold;
        utility::IncrementTimer increment_timer;
        const utility::Lookup_Table_1D<float>::Points* torque_lookup_table_points;
        const std::size_t torque_lookup_table_size;
        const utility::Lookup_Table_1D<float>::Points* fan_request_lookup_table_points;
        const std::size_t fan_request_lookup_table_size;
    };

    EngineOverheatProtection(
        const Config config,
        const signal::FloatSignal oil_temp,
        const signal::BoolSignal is_engine_running,
        manager::Manager manager
    );

    void Update() override;

    const signal::BoolSignal& IsOverheatProtectedRef() const;

    const signal::FloatSignal& TorqueLimit() const;

    const signal::FloatSignal& FanRequestRef() const;

private:
    const signal::FloatSignal oil_temp_;
    const signal::BoolSignal is_engine_running_;

    engine_overheat_protection::EngineOverheatProtectionState state_;

    const signal::BoolSignal is_overheat_protection_output_;
    const signal::FloatSignal torque_limit_output_;
    const signal::FloatSignal fan_request_output_;

    utility::IncrementTimer increment_timer_;
    utility::Lookup_Table_1D<float>::Points torque_lookup_table_points_;
    utility::Lookup_Table_1D<float>::Points fan_request_lookup_table_points_;

};
}

#endif
