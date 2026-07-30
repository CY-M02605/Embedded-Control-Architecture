/**
 * @file engine_overheat_protection.cc
 * @brief create a source file to realise header file
 * @date 2026-07-29
 */

#include "engine_overheat_protection.h"

using engine_overheat_protection::EngineOverheatProtection;

EngineOverheatProtection::EngineOverheatProtection (
    const Config& config,
    const signals::FloatSignal& oil_temp,
    const signals::BoolSignal& is_engine_running,
    framework::Manager& manager
): config_(config),
   oil_temp_(oil_temp),
   is_engine_running_(is_engine_running),
   state_(engine_overheat_protection::EngineOverheatProtectionState::IDLE),
   is_overheat_protection_output_(false, signals::ValidityStatus::INVALID),
   torque_limit_output_(0.0f, signals::ValidityStatus::INVALID),
   fan_request_output_(0.0f, signals::ValidityStatus::INVALID),
   increment_timer_(config.increment_timer_config),
   torque_lookup_table_(
    config.torque_lookup_table_points,
    config.torque_lookup_table_size
   ),
   fan_request_lookup_table_(
    config.fan_request_lookup_table_points,
    config.fan_request_lookup_table_size
   ) {
   manager.RegisterModule(this); 
}

void EngineOverheatProtection::Update() {
    bool is_overheat_protection_alarm_output = false;
    signals::ValidityStatus alarm_validity = signals::ValidityStatus::INVALID;

    float torque_limit_output = 0.0f;
    signals::ValidityStatus torque_limit_validity = signals::ValidityStatus::INVALID;

    float fan_request_output = 0.0f;
    signals::ValidityStatus fan_request_validity = signals::ValidityStatus::INVALID;

    switch (state_)
    {
    case EngineOverheatProtectionState::IDLE:
        if (!oil_temp_.IsValid() || !is_engine_running_.IsValid()) {

            bool is_overheat_protection_alarm_output = false;
            signals::ValidityStatus alarm_validity = signals::ValidityStatus::INVALID;

            float torque_limit_output = 0.0f;
            signals::ValidityStatus torque_limit_validity = signals::ValidityStatus::INVALID;

            float fan_request_output = 0.0f;
            signals::ValidityStatus fan_request_validity = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::IDLE;
            increment_timer_.Clear();

            break;

        }

        if (is_engine_running_.GetValue() && oil_temp_.GetValue() >= config_.oil_high_threshold) {

            float torque_limit_output = torque_lookup_table_.LookupTable(oil_temp_.GetValue());

            float fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());

            state_ = EngineOverheatProtectionState::COUNTING;

        } else if (!is_engine_running_.GetValue() && oil_temp_.GetValue() >= config_.oil_high_threshold) {

            float torque_limit_output = 0.0f;

            float fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());

            state_ = EngineOverheatProtectionState::AFTER_RUN_COOLING;

        } else {

            float torque_limit_output = torque_lookup_table_.LookupTable(oil_temp_.GetValue());

            float fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());

            state_ = EngineOverheatProtectionState::IDLE;

        }

        bool is_overheat_protection_alarm_output = false;
        signals::ValidityStatus alarm_validity = signals::ValidityStatus::VALID;

        signals::ValidityStatus torque_limit_validity = signals::ValidityStatus::VALID;

        signals::ValidityStatus fan_request_validity = signals::ValidityStatus::VALID;

        increment_timer_.Clear();

    
    case EngineOverheatProtectionState::COUNTING:
        /* code */
        break;

    case EngineOverheatProtectionState::PROTECTED:
        /* code */
        break;

    case EngineOverheatProtectionState::AFTER_RUN_COOLING:
        /* code */
        break;
    }

}
