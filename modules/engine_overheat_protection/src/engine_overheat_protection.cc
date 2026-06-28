/**
 * @file engine_overheat_protection.cc
 * @brief EngineOverheatProtection in normal modules as timer-driven state machine pattern
 * @date 2026-06-26
 */

#include "engine_overheat_protection.h"

using engine_overheat_protection::EngineOverheatProtection;

EngineOverheatProtection::EngineOverheatProtection(
    const Config &config,
    const signals::FloatSignal &oil_temp,
    const signals::BoolSignal &is_engine_running,
    framework::Manager &manager) : config_(config),
                                   oil_temp_(oil_temp),
                                   is_engine_running_(is_engine_running),
                                   is_overheat_protected_output_(false, signals::ValidityStatus::VALID),
                                   increment_timer_(config.increment_timer_config),
                                   state_(EngineOverheatProtectionState::IDLE)
{
    manager.RegisterModule(*this);
}

void EngineOverheatProtection::Update()
{

    bool output_;
    signals::ValidityStatus validation;

    switch (state_)
    {
    case EngineOverheatProtectionState::IDLE:
        if (!is_engine_running_.IsValid() || !oil_temp_.IsValid())
        {
            output_ = false;
            validation = signals::ValidityStatus::INVALID;
            state_ = EngineOverheatProtectionState::IDLE;
            break;
        }

        if (is_engine_running_.GetValue() && oil_temp_.GetValue() >= config_.oil_high_threshold) {
            output_ = false;
            validation = signals::ValidityStatus::VALID;
            state_ = EngineOverheatProtectionState::COUNTING;
            increment_timer_.Clear();
        } else {
            output_ = false;
            validation = signals::ValidityStatus::VALID;
            state_ = EngineOverheatProtectionState::IDLE;
        }
        break;

    case EngineOverheatProtectionState::COUNTING:
        if (!is_engine_running_.IsValid() || !oil_temp_.IsValid())
        {
            output_ = false;
            validation = signals::ValidityStatus::INVALID;
            state_ = EngineOverheatProtectionState::IDLE;
            break;
        }

        if (oil_temp_.GetValue() < config_.oil_low_threshold)
        {
            output_ = false;
            validation = signals::ValidityStatus::VALID;
            state_ = EngineOverheatProtectionState::IDLE;
            break;
        }

        increment_timer_.Update();

        if (increment_timer_.IsTimeUp())
        {
            output_ = true;
            validation = signals::ValidityStatus::VALID;
            state_ = EngineOverheatProtectionState::PROTECTED;
            increment_timer_.Clear();
        }
        else
        {
            output_ = false;
            validation = signals::ValidityStatus::VALID;
        }
        break;

    case EngineOverheatProtectionState::PROTECTED:
        if (!is_engine_running_.IsValid() || !oil_temp_.IsValid())
        {
            output_ = false;
            validation = signals::ValidityStatus::INVALID;
            state_ = EngineOverheatProtectionState::IDLE;
            break;
        }

        if (oil_temp_.GetValue() < config_.oil_low_threshold)
        {
            output_ = false;
            validation = signals::ValidityStatus::VALID;
            state_ = EngineOverheatProtectionState::IDLE;
            increment_timer_.Clear();
        } else {
            output_ = false;
            validation = signals::ValidityStatus::VALID;
        }
        break;

    }

    is_overheat_protected_output_ = signals::BoolSignal(output_, validation);
}

const signals::BoolSignal &EngineOverheatProtection::IsOverheatProtectedRef() const
{
    return is_overheat_protected_output_;
}
