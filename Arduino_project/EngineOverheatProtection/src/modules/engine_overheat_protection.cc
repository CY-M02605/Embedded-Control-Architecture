/**
 * @file engine_overheat_protection.cc
 * @brief create a source file to realise header file
 * @date 2026-07-29
 */

#include "engine_overheat_protection.h"

// #include <iostream>

using engine_overheat_protection::EngineOverheatProtection;

using engine_overheat_protection::EngineOverheatProtectionState;

EngineOverheatProtection::EngineOverheatProtection (
    const Config& config,
    const signals::FloatSignal& oil_temp,
    const signals::BoolSignal& is_engine_running,
    const signals::BoolSignal& clear_fault_request,
    framework::Manager& manager
):config_(config), oil_temp_(oil_temp), is_engine_running_(is_engine_running), clear_fault_request_(clear_fault_request),
state_(EngineOverheatProtectionState::STOP),
fault_reason_(FaultReason::NONE),
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
    case EngineOverheatProtectionState::STOP: { 
        const bool is_fault_condition = 
            !is_engine_running_.IsValid() ||
            !oil_temp_.IsValid() ||
            oil_temp_.GetValue() >= config_.oil_temp_physical_max ||
            oil_temp_.GetValue() >= config_.oil_temp_high_threshold ||
            oil_temp_.GetValue() <= config_.oil_temp_physical_min;

        if (is_fault_condition) {
            if (!is_engine_running_.IsValid()) {

                fault_reason_ = FaultReason::ENGINE_RUNNING_SIGNAL_INVALID;
            
            } else if (!oil_temp_.IsValid()) {
            
                fault_reason_ = FaultReason::OIL_TEMP_SIGNAL_INVALID;
            
            } else if (oil_temp_.GetValue() >= config_.oil_temp_physical_max) {

                fault_reason_ = FaultReason::TEMP_OUT_OF_RANGE_HIGH;

            } else if (oil_temp_.GetValue() >= config_.oil_temp_high_threshold) {

                fault_reason_ = FaultReason::UNEXPECTED_HIGH_TEMP_IN_STOP;

            } else if (oil_temp_.GetValue() <= config_.oil_temp_physical_min) {

                fault_reason_ = FaultReason::TEMP_OUT_OF_RANGE_LOW;

            } 

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::INVALID;

            torque_limit_output = 0.0f;
            torque_limit_validity = signals::ValidityStatus::INVALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            
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
            fault_reason_ = FaultReason::NONE;
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
        fault_reason_ = FaultReason::NONE;
        increment_timer_.Clear();

        break;
    }
    
    case EngineOverheatProtectionState::IDLE: { 
        const bool is_fault_condition = 
            !is_engine_running_.IsValid() ||
            !oil_temp_.IsValid() ||
            oil_temp_.GetValue() >= config_.oil_temp_physical_max ||
            oil_temp_.GetValue() <= config_.oil_temp_physical_min;

        if (is_fault_condition) {
            if (!is_engine_running_.IsValid()) {

                fault_reason_ = FaultReason::ENGINE_RUNNING_SIGNAL_INVALID;
            
            } else if (!oil_temp_.IsValid()) {
            
                fault_reason_ = FaultReason::OIL_TEMP_SIGNAL_INVALID;
            
            } else if (oil_temp_.GetValue() >= config_.oil_temp_physical_max) {

                fault_reason_ = FaultReason::TEMP_OUT_OF_RANGE_HIGH;

            } else if (oil_temp_.GetValue() <= config_.oil_temp_physical_min) {

                fault_reason_ = FaultReason::TEMP_OUT_OF_RANGE_LOW;

            } 

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::INVALID;

            torque_limit_output = 0.0f;
            torque_limit_validity = signals::ValidityStatus::INVALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            
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
            fault_reason_ = FaultReason::NONE;
            increment_timer_.Clear();

            break;
        }

        if (oil_temp_.GetValue() >= config_.oil_temp_high_threshold) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::COUNTING;
            fault_reason_ = FaultReason::NONE;
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
        fault_reason_ = FaultReason::NONE;
        increment_timer_.Clear();

        break;
    }

    case EngineOverheatProtectionState::COUNTING: { 
        const bool is_fault_condition = 
            !is_engine_running_.IsValid() ||
            !oil_temp_.IsValid() ||
            oil_temp_.GetValue() >= config_.oil_temp_physical_max ||
            oil_temp_.GetValue() <= config_.oil_temp_physical_min;

        if (is_fault_condition) {
            if (!is_engine_running_.IsValid()) {

                fault_reason_ = FaultReason::ENGINE_RUNNING_SIGNAL_INVALID;
            
            } else if (!oil_temp_.IsValid()) {
            
                fault_reason_ = FaultReason::OIL_TEMP_SIGNAL_INVALID;
            
            } else if (oil_temp_.GetValue() >= config_.oil_temp_physical_max) {

                fault_reason_ = FaultReason::TEMP_OUT_OF_RANGE_HIGH;

            } else if (oil_temp_.GetValue() <= config_.oil_temp_physical_min) {

                fault_reason_ = FaultReason::TEMP_OUT_OF_RANGE_LOW;

            } 

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::INVALID;

            torque_limit_output = 0.0f;
            torque_limit_validity = signals::ValidityStatus::INVALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            
            increment_timer_.Clear();

            break;    
        }

        if (oil_temp_.GetValue() <= config_.oil_temp_low_threshold) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = 100.0f;
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = 20.0f;
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::IDLE;
            fault_reason_ = FaultReason::NONE;
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
            fault_reason_ = FaultReason::NONE;
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
            fault_reason_ = FaultReason::NONE;
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
        fault_reason_ = FaultReason::NONE;

        break;        
    }

    case EngineOverheatProtectionState::PROTECTED: {
        const bool is_fault_condition = 
            !is_engine_running_.IsValid() ||
            !oil_temp_.IsValid() ||
            oil_temp_.GetValue() >= config_.oil_temp_physical_max ||
            oil_temp_.GetValue() <= config_.oil_temp_physical_min;

        if (is_fault_condition) {
            if (!is_engine_running_.IsValid()) {

                fault_reason_ = FaultReason::ENGINE_RUNNING_SIGNAL_INVALID;
            
            } else if (!oil_temp_.IsValid()) {
            
                fault_reason_ = FaultReason::OIL_TEMP_SIGNAL_INVALID;
            
            } else if (oil_temp_.GetValue() >= config_.oil_temp_physical_max) {

                fault_reason_ = FaultReason::TEMP_OUT_OF_RANGE_HIGH;

            } else if (oil_temp_.GetValue() <= config_.oil_temp_physical_min) {

                fault_reason_ = FaultReason::TEMP_OUT_OF_RANGE_LOW;

            } 

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::INVALID;

            torque_limit_output = 0.0f;
            torque_limit_validity = signals::ValidityStatus::INVALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            
            increment_timer_.Clear();

            break;    
        }

        if (oil_temp_.GetValue() <= config_.oil_temp_low_threshold) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = 100.0f;
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = 20.0f;
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::IDLE;
            fault_reason_ = FaultReason::NONE;
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
            fault_reason_ = FaultReason::NONE;
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
        fault_reason_ = FaultReason::NONE;
        increment_timer_.Clear();

        break;
    }

    case EngineOverheatProtectionState::AFTER_RUN_COOLING: { 
        const bool is_fault_condition = 
            !is_engine_running_.IsValid() ||
            !oil_temp_.IsValid() ||
            oil_temp_.GetValue() >= config_.oil_temp_physical_max ||
            oil_temp_.GetValue() <= config_.oil_temp_physical_min;

        if (is_fault_condition) {
            if (!is_engine_running_.IsValid()) {

                fault_reason_ = FaultReason::ENGINE_RUNNING_SIGNAL_INVALID;
            
            } else if (!oil_temp_.IsValid()) {
            
                fault_reason_ = FaultReason::OIL_TEMP_SIGNAL_INVALID;
            
            } else if (oil_temp_.GetValue() >= config_.oil_temp_physical_max) {

                fault_reason_ = FaultReason::TEMP_OUT_OF_RANGE_HIGH;

            } else if (oil_temp_.GetValue() <= config_.oil_temp_physical_min) {

                fault_reason_ = FaultReason::TEMP_OUT_OF_RANGE_LOW;

            } 

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::INVALID;

            torque_limit_output = 0.0f;
            torque_limit_validity = signals::ValidityStatus::INVALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            
            increment_timer_.Clear();

            break;    
        }
        
        if (oil_temp_.GetValue() <= config_.oil_temp_low_threshold) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = 100.0f;
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::STOP;
            fault_reason_ = FaultReason::NONE;
            increment_timer_.Clear();

            break;
        }

        if (is_engine_running_.GetValue() && oil_temp_.GetValue() >= config_.oil_temp_high_threshold) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = torque_lookup_table_.LookupTable(oil_temp_.GetValue());
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = fan_request_lookup_table_.LookupTable(oil_temp_.GetValue());
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::COUNTING;
            fault_reason_ = FaultReason::NONE;
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
        fault_reason_ = FaultReason::NONE;
        increment_timer_.Clear();

        break;
    }

    case EngineOverheatProtectionState::FAULT: { 
        const bool can_clear_fault_conditions = 
            clear_fault_request_.IsValid() && 
            clear_fault_request_.GetValue() &&
            oil_temp_.IsValid() && 
            oil_temp_.GetValue() <= config_.oil_temp_low_threshold &&
            oil_temp_.GetValue() >= config_.oil_temp_low_fault_recover_critical_value && 
            is_engine_running_.IsValid() &&
            !is_engine_running_.GetValue();

        // std::cout << "can_clear_fault_conditions = " << can_clear_fault_conditions << std::endl;
        // bool x = clear_fault_request_.GetValue();
        // std::cout << "x = " << x << std::endl;

        const bool is_clearable_fault = 
            fault_reason_ == FaultReason::ENGINE_RUNNING_SIGNAL_INVALID ||
            fault_reason_ == FaultReason::OIL_TEMP_SIGNAL_INVALID ||
            fault_reason_ == FaultReason::TEMP_OUT_OF_RANGE_HIGH ||
            fault_reason_ == FaultReason::TEMP_OUT_OF_RANGE_LOW ||
            // fault_reason_ == FaultReason::TEMP_RISE_TOO_FAST ||
            fault_reason_ == FaultReason::UNEXPECTED_HIGH_TEMP_IN_STOP;
        
        // std::cout << "is_clearable_fault = " << is_clearable_fault << std::endl;

        if (can_clear_fault_conditions && is_clearable_fault) {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::VALID;

            torque_limit_output = 100.0f;
            torque_limit_validity = signals::ValidityStatus::VALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::VALID;

            state_ = EngineOverheatProtectionState::STOP;
            fault_reason_ = FaultReason::NONE;
            increment_timer_.Clear();

        } else {

            is_overheat_protection_output = false;
            is_overheat_protection_validity = signals::ValidityStatus::INVALID;

            torque_limit_output = 0.0f;
            torque_limit_validity = signals::ValidityStatus::INVALID;

            fan_request_output = 0.0f;
            fan_request_validity = signals::ValidityStatus::INVALID;

            state_ = EngineOverheatProtectionState::FAULT;
            increment_timer_.Clear();

        }
        break;
    }

    default:

        is_overheat_protection_output = false;
        is_overheat_protection_validity = signals::ValidityStatus::VALID;

        torque_limit_output = 100.0f;
        torque_limit_validity = signals::ValidityStatus::VALID;

        fan_request_output = 0.0f;
        fan_request_validity = signals::ValidityStatus::VALID;

        state_ = EngineOverheatProtectionState::STOP;
        fault_reason_ = FaultReason::NONE;
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

const engine_overheat_protection::FaultReason EngineOverheatProtection::FaultReasonRef() const {
    return fault_reason_;
}
