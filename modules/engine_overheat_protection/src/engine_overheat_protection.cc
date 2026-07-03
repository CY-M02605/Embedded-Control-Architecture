/**
 * @file engine_overheat_protection.cc
 * @brief EngineOverheatProtection in normal modules as timer-driven state machine pattern
 * @date 2026-06-26
 */

#include "engine_overheat_protection.h"

using engine_overheat_protection::EngineOverheatProtection;

EngineOverheatProtection::EngineOverheatProtection(
    const Config& config,
    const signals::FloatSignal& oil_temp,
    const signals::BoolSignal& is_engine_running,
    framework::Manager &manager) : config_(config),
    oil_temp_(oil_temp),is_engine_running_(is_engine_running),
    is_overheat_protected_output_(false, signals::ValidityStatus::VALID),
    torque_limit_output_(0.0f, signals::ValidityStatus::INVALID),
    fan_request_output_(0.0, signals::ValidityStatus::INVALID),
    increment_timer_(config.increment_timer_config),
    torque_lookup_table_(
        config.torque_lookup_table_points, 
        config.torque_lookup_table_size
    ),
    fan_lookup_table_(
        config.fan_lookup_table_points, 
        config.fan_lookup_table_size
    ),
    state_(EngineOverheatProtectionState::IDLE) {
        manager.RegisterModule(*this);
}

void EngineOverheatProtection::Update()
{

    bool alarm_output = false;
    signals::ValidityStatus alarm_validation = signals::ValidityStatus::INVALID;

    float t_d_output = 0.0f;
    signals::ValidityStatus t_d_validation = signals::ValidityStatus::INVALID;

    float fan_output = 0.0f;
    signals::ValidityStatus fan_validation = signals::ValidityStatus::INVALID;

    switch (state_)
    {
    case EngineOverheatProtectionState::IDLE:
        if (!is_engine_running_.IsValid() || !oil_temp_.IsValid()) {

            alarm_output = false;
            alarm_validation = signals::ValidityStatus::INVALID;

            t_d_output = 0.0f;
            t_d_validation = signals::ValidityStatus::INVALID;

            fan_output = 0.0f;
            fan_validation = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::IDLE;
            increment_timer_.Clear();

            break;
        }

        if (is_engine_running_.GetValue() && oil_temp_.GetValue() >= config_.oil_high_threshold) {

            state_ = EngineOverheatProtectionState::COUNTING;

        } else {

            state_ = EngineOverheatProtectionState::IDLE;
            
        }

        alarm_output = false;
        alarm_validation = signals::ValidityStatus::VALID;

        t_d_output = 100.0f;
        t_d_validation = signals::ValidityStatus::VALID;

        fan_output = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
        fan_validation = signals::ValidityStatus::VALID;

        increment_timer_.Clear();

        break;

    case EngineOverheatProtectionState::COUNTING:
        if (!is_engine_running_.IsValid() || !oil_temp_.IsValid()) {

            alarm_output = false;
            alarm_validation = signals::ValidityStatus::INVALID;

            t_d_output = 0.0f;
            t_d_validation = signals::ValidityStatus::INVALID;

            fan_output = 0.0f;
            fan_validation = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::IDLE;
            increment_timer_.Clear();

            break;
        }

        if (!is_engine_running_.GetValue()) {

            alarm_output = false;
            alarm_validation = signals::ValidityStatus::VALID;

            t_d_output = 0.0f;
            t_d_validation = signals::ValidityStatus::VALID;

            fan_output = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::IDLE;
            increment_timer_.Clear();

            break;
        }

        if (oil_temp_.GetValue() <= config_.oil_low_threshold) {

            alarm_output = false;
            alarm_validation = signals::ValidityStatus::VALID;

            t_d_output = 100.0f;
            t_d_validation = signals::ValidityStatus::VALID;

            fan_output = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::IDLE;
            increment_timer_.Clear();

            break;
        }

        increment_timer_.Update();

        if (increment_timer_.IsTimeUp()) {

            alarm_output = true;
            alarm_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::PROTECTED;
            increment_timer_.Clear();

        } else {

            alarm_output = false;
            alarm_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::COUNTING;

        }

        t_d_output = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
        t_d_validation = signals::ValidityStatus::VALID;

        fan_output = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
        fan_validation = signals::ValidityStatus::VALID;

        break;

    case EngineOverheatProtectionState::PROTECTED:
        if (!is_engine_running_.IsValid() || !oil_temp_.IsValid()) {

            alarm_output = false;
            alarm_validation = signals::ValidityStatus::INVALID;

            t_d_output = 0.0f;
            t_d_validation = signals::ValidityStatus::INVALID;

            fan_output = 0.0f;
            fan_validation = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::IDLE;
            increment_timer_.Clear();

            break;
        }

        if (oil_temp_.GetValue() <= config_.oil_low_threshold) {

            alarm_output = false;
            alarm_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::IDLE;
            increment_timer_.Clear();

        } else {

            alarm_output = true;
            alarm_validation = signals::ValidityStatus::VALID;
        
        }

        t_d_output = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
        t_d_validation = signals::ValidityStatus::VALID;

        fan_output = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
        fan_validation = signals::ValidityStatus::VALID;

        break;

    }

    is_overheat_protected_output_ = signals::BoolSignal(alarm_output, alarm_validation);

    torque_limit_output_ = signals::FloatSignal(t_d_output, t_d_validation);

    fan_request_output_ = signals::FloatSignal(fan_output, fan_validation);

}

const signals::BoolSignal& EngineOverheatProtection::IsOverheatProtectedRef() const {
    return is_overheat_protected_output_;
}

const signals::FloatSignal& EngineOverheatProtection::TorqueLimitRef() const {
    return torque_limit_output_;
}

const signals::FloatSignal& EngineOverheatProtection::FanRequestRef() const {
    return fan_request_output_;
}
