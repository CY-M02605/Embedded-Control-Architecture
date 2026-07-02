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

#include "lookup_table_1d.h"

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
            utility::LookupTable1D<float>::Point* torque_lookup_table_points;
            std::size_t torque_lookup_table_size;
            utility::LookupTable1D<float>::Point* fan_lookup_table_points;
            std::size_t fan_lookup_table_size;
        };

        EngineOverheatProtection(
            const Config& config,
            const signals::FloatSignal& oil_temp,
            const signals::BoolSignal& is_engine_running,
            framework::Manager& manager
        );

        void Update() override;

        const signals::BoolSignal& IsOverheatProtectedRef() const;

        const signals::BoolSignal& IsOverheatTorqueDecreasingRef() const;

        const signals::BoolSignal& IsOverheatTorqueDecreasingRef() const;

    private:
        EngineOverheatProtectionState state_;

        const Config& config_;
        const signals::FloatSignal& oil_temp_;
        const signals::BoolSignal& is_engine_running_;
        signals::BoolSignal is_overheat_protected_output_;
        
        signals::FloatSignal is_overheat_torque_decreasing_output_;
        signals::FloatSignal is_overheat_fan_output_;
        
        utility::IncrementTimer increment_timer_;
        utility::LookupTable1D<float> torque_lookup_table_;
        utility::LookupTable1D<float> fan_lookup_table_;

};
}

#endif
