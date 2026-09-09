/**
 * @file engine_overheat_protection.h
 * @brief EngineOverheatProtection in normal modules as timer-driven state machine pattern
 * @date 2026-06-26 (2026-09-02)
 */

#ifndef ENGINE_OVERHEAT_PROTECTION_H
#define ENGINE_OVERHEAT_PROTECTION_H

#include <cstddef>

#include "manager.h"
#include "module_interface.h"
#include "signal.h"

#include "increment_timer.h"
#include "lookup_table_1d.h"
#include "rate_of_change_checker.h"

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
    TEMP_OUT_OF_RANGE_HIGH,
    UNEXPECTED_HIGH_TEMP_IN_STOP,
    TEMP_OUT_OF_RANGE_LOW,
    TEMP_RISE_TOO_FAST,
};

class EngineOverheatProtection: public framework::ModuleInterface {
    public:
        struct Config {
            float oil_low_threshold;
            float oil_high_threshold;
            float oil_temp_physical_max;
            float oil_temp_physical_min;
            float oil_temp_low_fault_recover_critical_value;
            utility::IncrementTimer::Config increment_timer_config;
            utility::RateOfChangeChecker::Config max_rate_of_change_config;
            const utility::LookupTable1D<float>::Point* torque_lookup_table_points;
            std::size_t torque_lookup_table_size;
            const utility::LookupTable1D<float>::Point* fan_lookup_table_points;
            std::size_t fan_lookup_table_size;
        };

        EngineOverheatProtection(
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

        const float GetCurrentRate() const;

        const engine_overheat_protection::EngineOverheatProtectionState StateRef() const;

        const engine_overheat_protection::FaultReason FaultReasonRef() const;

    private:

        const Config& config_;
        const signals::FloatSignal& oil_temp_;
        const signals::BoolSignal& is_engine_running_;
        const signals::BoolSignal clear_fault_request_;

        signals::BoolSignal is_overheat_protected_output_;
        signals::FloatSignal torque_limit_output_;
        signals::FloatSignal fan_request_output_;

        engine_overheat_protection::EngineOverheatProtectionState state_;
        engine_overheat_protection::FaultReason fault_reason_;
        float current_rate_of_change_;
        
        utility::IncrementTimer increment_timer_;
        utility::LookupTable1D<float> torque_lookup_table_;
        utility::LookupTable1D<float> fan_lookup_table_;
        utility::RateOfChangeChecker max_rate_of_change_;

};
}

#endif
