/**
 * @file engine_overheat_protection.h
 * @brief EngineOverheatProtection in normal modules as timer-driven state machine pattern
 * @date 2026-06-26
 */

#ifndef ENGINE_OVERHEAT_PROTECTION_H
#define ENGINE_OVERHEAT_PROTECTION_H

#include "manager.h"
#include "module_interface.h"
#include "signal.h"

#include "increment_timer.h"

namespace engine_overheat_protection {

enum class EngineOverheatProtectionState{
    IDLE,
    COUNTING,
    PROTECTED,
    // AFTER_RUN_COOLING
};

class EngineOverheatProtection: public framework::ModuleInterface {
    public:
        struct Config {
            float oil_low_threshold;
            float oil_high_threshold;
            utility::IncrementTimer::Config increment_timer_config;
        };

        EngineOverheatProtection(
            const Config& config,
            const signals::FloatSignal& oil_temp,
            const signals::BoolSignal& is_engine_running,
            framework::Manager& manager
        );

        void Update() override;

        const signals::BoolSignal& IsOverheatProtectedRef() const;

    private:
        EngineOverheatProtectionState state_;

        const Config& config_;
        const signals::FloatSignal& oil_temp_;
        const signals::BoolSignal& is_engine_running_;
        signals::BoolSignal is_overheat_protected_output_;
        utility::IncrementTimer increment_timer_;

};
}

#endif
