/**
 * @file engine_overheat_protection.cc
 * @brief create a source file to realise header file
 * @date 2026-07-29
 */

#include "engine_overheat_protection.h"

using engine_overheat_protection::EngineOverheatProtection;

using engine_overheat_protection::EngineOverheatProtectionState;

EngineOverheatProtection::EngineOverheatProtection (
    const Config& config,
    const signals::FloatSignal& oil_temp,
    const signals::BoolSignal& is_engine_running,
    framework::Manager& manager
):config_(config), oil_temp_(oil_temp), is_engine_running_(is_engine_running), 
state_(EngineOverheatProtectionState::STOP),
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
    bool is_overheat_protection_output = false;
    signals::ValidityStatus is_overheat_protection_validity = signals::ValidityStatus::INVALID;

    float torque_limit_output = 0.0f;
    signals::ValidityStatus torque_limit_validity = signals::ValidityStatus::INVALID;

    float fan_request_output = 0.0f;
    signals::ValidityStatus fan_request_validity = signals::ValidityStatus::INVALID;

    switch (state_)
    {
    case EngineOverheatProtectionState::STOP:
        if (!is_engine_running_.IsValid() || !oil_temp_.IsValid()) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::INVALID;

            torque_limit_output = 0.0f;
            torque_limit_validity = signals::ValidityStatus::INVALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::STOP;
            increment_timer_.Clear();

            break;
        }

        if (is_engine_running_.GetValue()) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = 100.0f;
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::IDLE;
            increment_timer_.Clear();

            break;
        }

        is_overheat_protection_output = false;
        is_overheat_protection_validity = signals::ValidityStatus::VALID;

        torque_limit_output = 100.0f;
        torque_limit_validity = signals::ValidityStatus::VALID;

        fan_request_output = 0.0f;
        fan_request_validity = signals::ValidityStatus::VALID;

        state_ = EngineOverheatProtectionState::STOP;
        increment_timer_.Clear();

        break;
    
    case EngineOverheatProtectionState::IDLE:
        if (!oil_temp_.IsValid() || !is_engine_running_.IsValid()) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::INVALID;

            torque_limit_output = 0.0f;
            torque_limit_validity = signals::ValidityStatus::INVALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::STOP;
            increment_timer_.Clear();

            break;
        }

        if (!is_engine_running_.GetValue()) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = 100.0f;
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::STOP;
            increment_timer_.Clear();

            break;
        }

        if (oil_temp_.GetValue() >= config_.oil_high_threshold) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::COUNTING;
            increment_timer_.Clear();

            break;
        }

        is_overheat_protection_output = false;
        is_overheat_protection_validity = signals::ValidityStatus::VALID;

        torque_limit_output = 100.0f;
        torque_limit_validity = signals::ValidityStatus::VALID;

        fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());
        fan_request_validity = signals::ValidityStatus::VALID;

        state_ = EngineOverheatProtectionState::IDLE;
        increment_timer_.Clear();

        break;

    case EngineOverheatProtectionState::COUNTING:

        if (!is_engine_running_.IsValid() || !oil_temp_.IsValid()) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::INVALID;

            torque_limit_output = 0.0f;
            torque_limit_validity = signals::ValidityStatus::INVALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::STOP;
            increment_timer_.Clear();

            break;
        }

        if (oil_temp_.GetValue() <= config_.oil_low_threshold) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = 100.0f;
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = 20.0f;
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::IDLE;
            increment_timer_.Clear();

            break;
        } 

        if (!is_engine_running_.GetValue()) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = 100.0f;
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::AFTER_RUN_COOLING;
            increment_timer_.Clear();

            break;
        }

        increment_timer_.Update();
           
        if (increment_timer_.IsTimeUp()) {

            is_overheat_protection_output = true;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::PROTECTED;
            increment_timer_.Clear();

            break;
        }

        is_overheat_protection_output = false;
        is_overheat_protection_validity = signals::ValidityStatus::VALID;

        torque_limit_output = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
        torque_limit_validity = signals::ValidityStatus::VALID;

        fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());
        fan_request_validity = signals::ValidityStatus::VALID;

        state_ = EngineOverheatProtectionState::COUNTING;

        break;        

    case EngineOverheatProtectionState::PROTECTED:
        
        if (!is_engine_running_.IsValid() || !oil_temp_.IsValid()) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::INVALID;

            torque_limit_output = 0.0f;
            torque_limit_validity = signals::ValidityStatus::INVALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::STOP;
            increment_timer_.Clear();

            break;
        }

        if (oil_temp_.GetValue() <= config_.oil_low_threshold) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = 100.0f;
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = 20.0f;
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::IDLE;
            increment_timer_.Clear();

            break;
        }
            
        if (!is_engine_running_.GetValue()) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = 100.0f;
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::AFTER_RUN_COOLING;
            increment_timer_.Clear();

            break;
        }

        is_overheat_protection_output = true;
        is_overheat_protection_validity = signals::ValidityStatus::VALID;

        torque_limit_output = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
        torque_limit_validity = signals::ValidityStatus::VALID;

        fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());
        fan_request_validity = signals::ValidityStatus::VALID;

        state_ = EngineOverheatProtectionState::PROTECTED;
        increment_timer_.Clear();

        break;

    case EngineOverheatProtectionState::AFTER_RUN_COOLING:

        if (!is_engine_running_.IsValid() || !oil_temp_.IsValid()) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::INVALID;

            torque_limit_output = 0.0f;
            torque_limit_validity = signals::ValidityStatus::INVALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::STOP;
            increment_timer_.Clear();

            break;
        }
        
        if (oil_temp_.GetValue() <= config_.oil_low_threshold) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = 100.0f;
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::STOP;
            increment_timer_.Clear();

            break;
        }

        if (is_engine_running_.GetValue() && oil_temp_.GetValue() >= config_.oil_high_threshold) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::COUNTING;
            increment_timer_.Clear();

            break;
        }

        is_overheat_protection_output = false;
        is_overheat_protection_validity = signals::ValidityStatus::VALID;

        torque_limit_output = 100.0f;
        torque_limit_validity = signals::ValidityStatus::VALID;

        fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());
        fan_request_validity = signals::ValidityStatus::VALID;

        state_ = EngineOverheatProtectionState::AFTER_RUN_COOLING;
        increment_timer_.Clear();

        break;

    default:

        is_overheat_protection_output = false;
        is_overheat_protection_validity = signals::ValidityStatus::INVALID;

        torque_limit_output = 0.0f;
        torque_limit_validity = signals::ValidityStatus::INVALID;

        fan_request_output = 0.0f;
        fan_request_validity = signals::ValidityStatus::INVALID;

        state_ = EngineOverheatProtectionState::STOP;
        increment_timer_.Clear();

        break;
    }

    is_overheat_protection_output_ = signals::BoolSignal(is_overheat_protection_output, is_overheat_protection_validity);
    torque_limit_output_ = signals::FloatSignal(torque_limit_output, torque_limit_validity);
    fan_request_output_ = signals::FloatSignal(fan_request_output, fan_request_validity);
}

const signals::BoolSignal& EngineOverheatProtection::IsOverheatProtectedRef() const {
    return is_overheat_protection_output_;
}

const signals::FloatSignal& EngineOverheatProtection::TorqueLimitRef() const {
    return torque_limit_output_;
}

const signals::FloatSignal& EngineOverheatProtection::FanRequestRef() const {
    return fan_request_output_;
}

const engine_overheat_protection::EngineOverheatProtectionState EngineOverheatProtection::StateRef() const {
    return state_;
}
