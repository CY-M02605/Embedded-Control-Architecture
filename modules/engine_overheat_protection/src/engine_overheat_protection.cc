/**
 * @file engine_overheat_protection.cc
 * @brief EngineOverheatProtection in normal modules as timer-driven state machine pattern
 * @date 2026-06-26 (2026-09-02)
 */

#include "engine_overheat_protection.h"

using engine_overheat_protection::EngineOverheatProtection;

EngineOverheatProtection::EngineOverheatProtection(
    const Config& config,
    const signals::FloatSignal& oil_temp,
    const signals::BoolSignal& is_engine_running,
    const signals::BoolSignal& clear_fault_request,
    framework::Manager& manager) : config_(config),
    oil_temp_(oil_temp),is_engine_running_(is_engine_running),clear_fault_request_(clear_fault_request),
    is_overheat_protected_output_(false, signals::ValidityStatus::VALID),
    torque_limit_output_(0.0f, signals::ValidityStatus::INVALID),
    fan_request_output_(0.0f, signals::ValidityStatus::INVALID),
    state_(EngineOverheatProtectionState::STOP),
    fault_reason_(FaultReason::NONE),
    increment_timer_(config.increment_timer_config),
    max_rate_of_change_(config.max_rate_of_change_config),
    current_rate_of_change_(0.0f),
    torque_lookup_table_(
        config.torque_lookup_table_points, 
        config.torque_lookup_table_size
    ),
    fan_lookup_table_(
        config.fan_lookup_table_points, 
        config.fan_lookup_table_size
    ){
        manager.RegisterModule(*this);
}

void EngineOverheatProtection::Update()
{

    bool is_engine_overheat_protected_value = false;
    signals::ValidityStatus is_engine_overheat_protected_validation = signals::ValidityStatus::INVALID;

    float torque_limit_value = 0.0f;
    signals::ValidityStatus torque_limit_validation = signals::ValidityStatus::INVALID;

    float fan_request_value = 0.0f;
    signals::ValidityStatus fan_request_validation = signals::ValidityStatus::INVALID;

    FaultReason detected_fault_reason = FaultReason::NONE;

    switch (state_)
    {
    case EngineOverheatProtectionState::STOP: {

        detected_fault_reason = FaultReason::NONE;

        if (!is_engine_running_.IsValid()) {
            detected_fault_reason = FaultReason::ENGINE_RUNNING_SIGNAL_INVALID;
        } else if (!oil_temp_.IsValid()) {
            detected_fault_reason = FaultReason::OIL_TEMP_SIGNAL_INVALID;
        } else if (oil_temp_.GetValue() >= config_.oil_temp_physical_max) {
            detected_fault_reason = FaultReason::TEMP_OUT_OF_RANGE_HIGH;
        } else if (oil_temp_.GetValue() >= config_.oil_high_threshold) {
            detected_fault_reason = FaultReason::UNEXPECTED_HIGH_TEMP_IN_STOP;
        } else if (oil_temp_.GetValue() <= config_.oil_temp_physical_min) {
            detected_fault_reason = FaultReason::TEMP_OUT_OF_RANGE_LOW;
        }

        if (detected_fault_reason != FaultReason::NONE) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::INVALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::INVALID;

            fan_request_value = 0.0f;
            fan_request_validation = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            fault_reason_ = detected_fault_reason;
            current_rate_of_change_ = 0.0f;

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

            break;
        }

        if (is_engine_running_.GetValue()) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = 100.0f;
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::IDLE;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = 0.0f;

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

        } else {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = 0.0f;
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::STOP;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = 0.0f;

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

        }
        break;
    }

    case EngineOverheatProtectionState::IDLE: {

        detected_fault_reason = FaultReason::NONE;

        if (!is_engine_running_.IsValid()) {
            detected_fault_reason = FaultReason::ENGINE_RUNNING_SIGNAL_INVALID;
        } else if (!oil_temp_.IsValid()) {
            detected_fault_reason = FaultReason::OIL_TEMP_SIGNAL_INVALID;
        } else if (oil_temp_.GetValue() >= config_.oil_temp_physical_max) {
            detected_fault_reason = FaultReason::TEMP_OUT_OF_RANGE_HIGH;
        } else if (oil_temp_.GetValue() <= config_.oil_temp_physical_min) {
            detected_fault_reason = FaultReason::TEMP_OUT_OF_RANGE_LOW;
        }

        if (detected_fault_reason != FaultReason::NONE) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::INVALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::INVALID;

            fan_request_value = 0.0f;
            fan_request_validation = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            fault_reason_ = detected_fault_reason;
            current_rate_of_change_ = 0.0f;

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

            break;
        }

        const bool oil_temp_rise_too_fast_flag =
            is_engine_running_.GetValue() &&
            max_rate_of_change_.CheckRateOfChange(
                oil_temp_.GetValue(),
                0.1f
        );

        if (oil_temp_rise_too_fast_flag) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::INVALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::INVALID;

            fan_request_value = 0.0f;
            fan_request_validation = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            fault_reason_ = FaultReason::TEMP_RISE_TOO_FAST;
            current_rate_of_change_ = max_rate_of_change_.GetCurrentRate();

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

            break;
        }

        if (!is_engine_running_.GetValue()) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = 0.0f;
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::STOP;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = 0.0f;

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

        } else if (oil_temp_.GetValue() >= config_.oil_high_threshold) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::COUNTING;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = max_rate_of_change_.GetCurrentRate();

            increment_timer_.Clear();

        } else {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::IDLE;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = max_rate_of_change_.GetCurrentRate();

            increment_timer_.Clear();

        }
        break;
    }   

    case EngineOverheatProtectionState::COUNTING: {

        detected_fault_reason = FaultReason::NONE;

        if (!is_engine_running_.IsValid()) {
            detected_fault_reason = FaultReason::ENGINE_RUNNING_SIGNAL_INVALID;
        } else if (!oil_temp_.IsValid()) {
            detected_fault_reason = FaultReason::OIL_TEMP_SIGNAL_INVALID;
        } else if (oil_temp_.GetValue() >= config_.oil_temp_physical_max) {
            detected_fault_reason = FaultReason::TEMP_OUT_OF_RANGE_HIGH;
        } else if (oil_temp_.GetValue() <= config_.oil_temp_physical_min) {
            detected_fault_reason = FaultReason::TEMP_OUT_OF_RANGE_LOW;
        }

        if (detected_fault_reason != FaultReason::NONE) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::INVALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::INVALID;

            fan_request_value = 0.0f;
            fan_request_validation = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            fault_reason_ = detected_fault_reason;
            current_rate_of_change_ = 0.0f;

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

            break;
        }

        const bool oil_temp_rise_too_fast_flag =
            is_engine_running_.GetValue() &&
            max_rate_of_change_.CheckRateOfChange(
                oil_temp_.GetValue(),
                0.1f
        );

        if (oil_temp_rise_too_fast_flag) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::INVALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::INVALID;

            fan_request_value = 0.0f;
            fan_request_validation = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            fault_reason_ = FaultReason::TEMP_RISE_TOO_FAST;
            current_rate_of_change_ = max_rate_of_change_.GetCurrentRate();

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

            break;
        }

        if (oil_temp_.GetValue() <= config_.oil_low_threshold) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::IDLE;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = max_rate_of_change_.GetCurrentRate();

            increment_timer_.Clear();

            break;
        } 

        if (increment_timer_.IsTimeUp()) {

            is_engine_overheat_protected_value = true;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::PROTECTED;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = max_rate_of_change_.GetCurrentRate();

            increment_timer_.Clear();

        } else {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::COUNTING;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = max_rate_of_change_.GetCurrentRate();

            increment_timer_.Update();

        }

        break;
    }
        
    case EngineOverheatProtectionState::PROTECTED: {

        detected_fault_reason = FaultReason::NONE;

        if (!is_engine_running_.IsValid()) {
            detected_fault_reason = FaultReason::ENGINE_RUNNING_SIGNAL_INVALID;
        } else if (!oil_temp_.IsValid()) {
            detected_fault_reason = FaultReason::OIL_TEMP_SIGNAL_INVALID;
        } else if (oil_temp_.GetValue() >= config_.oil_temp_physical_max) {
            detected_fault_reason = FaultReason::TEMP_OUT_OF_RANGE_HIGH;
        } else if (oil_temp_.GetValue() <= config_.oil_temp_physical_min) {
            detected_fault_reason = FaultReason::TEMP_OUT_OF_RANGE_LOW;
        }

        if (detected_fault_reason != FaultReason::NONE) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::INVALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::INVALID;

            fan_request_value = 0.0f;
            fan_request_validation = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            fault_reason_ = detected_fault_reason;
            current_rate_of_change_ = 0.0f;

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

            break;
        }

        const bool oil_temp_rise_too_fast_flag =
            is_engine_running_.GetValue() &&
            max_rate_of_change_.CheckRateOfChange(
                oil_temp_.GetValue(),
                0.1f
        );

        if (oil_temp_rise_too_fast_flag) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::INVALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::INVALID;

            fan_request_value = 0.0f;
            fan_request_validation = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            fault_reason_ = FaultReason::TEMP_RISE_TOO_FAST;
            current_rate_of_change_ = max_rate_of_change_.GetCurrentRate();

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

            break;
        }

        if (oil_temp_.GetValue() <= config_.oil_low_threshold) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::IDLE;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = max_rate_of_change_.GetCurrentRate();

            increment_timer_.Clear();

            break;
        } else if (!is_engine_running_.GetValue()) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::AFTER_RUN_COOLING;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = 0.0f;

            increment_timer_.Clear();

            break;
        } else {

            is_engine_overheat_protected_value = true;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::PROTECTED;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = max_rate_of_change_.GetCurrentRate();

            increment_timer_.Clear();

            break;
        }
    }

    case EngineOverheatProtectionState::AFTER_RUN_COOLING: {

        detected_fault_reason = FaultReason::NONE;

        if (!is_engine_running_.IsValid()) {
            detected_fault_reason = FaultReason::ENGINE_RUNNING_SIGNAL_INVALID;
        } else if (!oil_temp_.IsValid()) {
            detected_fault_reason = FaultReason::OIL_TEMP_SIGNAL_INVALID;
        } else if (oil_temp_.GetValue() >= config_.oil_temp_physical_max) {
            detected_fault_reason = FaultReason::TEMP_OUT_OF_RANGE_HIGH;
        } else if (oil_temp_.GetValue() <= config_.oil_temp_physical_min) {
            detected_fault_reason = FaultReason::TEMP_OUT_OF_RANGE_LOW;
        }

        if (detected_fault_reason != FaultReason::NONE) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::INVALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::INVALID;

            fan_request_value = 0.0f;
            fan_request_validation = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            fault_reason_ = detected_fault_reason;
            current_rate_of_change_ = 0.0f;

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

            break;
        }

        if (oil_temp_.GetValue() <= config_.oil_low_threshold) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = 0.0f;
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::STOP;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = 0.0f;

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

        } else if (is_engine_running_.GetValue()) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::COUNTING;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = max_rate_of_change_.GetCurrentRate();

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

        } else {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = fan_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::AFTER_RUN_COOLING;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = 0.0f;

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

        }
        break;
    }

    case EngineOverheatProtectionState::FAULT: {

        const bool can_clear_fault_conditions_for_first_level = 
            clear_fault_request_.IsValid() &&
            clear_fault_request_.GetValue() &&
            oil_temp_.IsValid() &&
            oil_temp_.GetValue() < config_.oil_low_threshold &&
            oil_temp_.GetValue() > config_.oil_temp_low_fault_recover_critical_value &&
            is_engine_running_.IsValid() &&
            !is_engine_running_.GetValue();

        const bool can_clear_fault_conditions_for_second_level = 
            clear_fault_request_.IsValid() &&
            clear_fault_request_.GetValue() &&
            oil_temp_.IsValid() &&
            oil_temp_.GetValue() > config_.oil_temp_physical_min &&
            oil_temp_.GetValue() < config_.oil_temp_physical_max &&
            is_engine_running_.IsValid() &&
            !is_engine_running_.GetValue(); 

        const bool is_clearable_faults_first_level = 
            fault_reason_ == FaultReason::ENGINE_RUNNING_SIGNAL_INVALID ||
            fault_reason_ == FaultReason::OIL_TEMP_SIGNAL_INVALID ||
            fault_reason_ == FaultReason::UNEXPECTED_HIGH_TEMP_IN_STOP ||
            fault_reason_ == FaultReason::TEMP_OUT_OF_RANGE_HIGH ||
            fault_reason_ == FaultReason::TEMP_OUT_OF_RANGE_LOW;

        const bool is_clearable_faults_second_level = 
            fault_reason_ == FaultReason::TEMP_RISE_TOO_FAST;
            

         if ((can_clear_fault_conditions_for_first_level && is_clearable_faults_first_level) ||
              (can_clear_fault_conditions_for_second_level && is_clearable_faults_second_level)) {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::VALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::VALID;

            fan_request_value = 0.0f;
            fan_request_validation = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::STOP;
            fault_reason_ = FaultReason::NONE;
            current_rate_of_change_ = 0.0f;
            increment_timer_.Clear();

            max_rate_of_change_.Reset();

        } else {

            is_engine_overheat_protected_value = false;
            is_engine_overheat_protected_validation = signals::ValidityStatus::INVALID;

            torque_limit_value = 0.0f;
            torque_limit_validation = signals::ValidityStatus::INVALID;

            fan_request_value = 0.0f;
            fan_request_validation = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            current_rate_of_change_ = 0.0f;

            increment_timer_.Clear();

            max_rate_of_change_.Reset();

        }
        break;
    }
    }

    is_overheat_protected_output_ = signals::BoolSignal(is_engine_overheat_protected_value, is_engine_overheat_protected_validation);

    torque_limit_output_ = signals::FloatSignal(torque_limit_value, torque_limit_validation);

    fan_request_output_ = signals::FloatSignal(fan_request_value, fan_request_validation);

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

const float EngineOverheatProtection::GetCurrentRate() const {
    return current_rate_of_change_;
}
