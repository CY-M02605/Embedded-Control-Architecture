/**
 * @file oil_temp_warning.cpp
 * @brief Declares the OilTempWarning module for Arduino UNO.
 * @date 2026-06-15
 */

#include "oil_temp_warning.h"

using modules::OilTempWarning;

OilTempWarning::OilTempWarning(
    const modules::OilTempWarning::Config config,
    const signals::FloatSignal& oil_temp,
    framework::Manager& manager
):config_(config),
oil_temp_(oil_temp),
warning_output_(false, signals::ValidityStatus::INVALID),
hysteresis_(config.hysteresis_config){
    manager.RegisterModule(*this);
}

void OilTempWarning::Update() {
    if (!oil_temp_.IsValid()) {
        warning_output_.Set(
            false,
            signals::ValidityStatus::INVALID
        );
        return;
    }

    bool warning_flag = hysteresis_.Update(oil_temp_.GetValue());

    // const bool warning_flag = 
    //     oil_temp_.GetValue() >= config_.high_threshold;
    
    warning_output_.Set(
        warning_flag,
        signals::ValidityStatus::VALID
    );
}

const signals::BoolSignal& 
OilTempWarning::WarningSignalRef() const {
    return warning_output_;
}
