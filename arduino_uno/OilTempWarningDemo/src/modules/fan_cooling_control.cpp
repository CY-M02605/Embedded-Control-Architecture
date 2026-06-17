/**
 * @file fan_cooling_control.cpp
 * @brief implement FanCoolingControl module for OILTEMPWARNINGDEMO project
 * @date 2026-06-17
 */

#include "fan_cooling_control.h"

using modules::FanCoolingControl;

FanCoolingControl::FanCoolingControl(
    const modules::FanCoolingControl::Config config,
    const signals::FloatSignal& oil_temp,
    framework::Manager& manager
):config_(config),
oil_temp_(oil_temp),
fan_cooling_output_(false, signals::ValidityStatus::INVALID),
hysteresis_({
    config.low_cooling_temp,
    config.high_cooling_temp
}){
    manager.RegisterModule(*this);
}

void FanCoolingControl::Update() {
    if (!oil_temp_.IsValid()) {
        fan_cooling_output_.Set(
            false,
            signals::ValidityStatus::INVALID
        );
        return;
    }

    bool flag = hysteresis_.Update(oil_temp_.GetValue());

    fan_cooling_output_.Set(
        flag,
        signals::ValidityStatus::VALID);
}

const signals::BoolSignal& FanCoolingControl::FanCoolingRef() const {
    return fan_cooling_output_;
}
