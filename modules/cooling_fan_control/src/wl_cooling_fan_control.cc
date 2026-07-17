/**
 * @file wl_cooling_fan_control.cc
 * @brief WlCoolingFanControl for wl_cooling_fan_control
 * @date 02.06.2026
 */

 #include "wl_cooling_fan_control.h"

 extern "C" {
    #include "wlCoolingFanControl.h"
 }

 using wl_cooling_fan_control::WlCoolingFanControl;

 WlCoolingFanControl::WlCoolingFanControl(
    const signals::TemperatureSignal& coolant_temp,
    const signals::PercentageSignal& engine_load,
    framework::Manager& manager
 ):coolant_temp_(coolant_temp),
 engine_load_(engine_load),
 fan_duty_output_(0.0f, signals::SignalValidity::VALID),
 over_heat_flag_output_(false, signals::SignalValidity::VALID) {
    manager.RegisterModule(*this);
 }

 void WlCoolingFanControl::Init() {
    INIT_wlCoolingFanControl();
 }

 void WlCoolingFanControl::Update() {
    Float32 coolantTemp = coolant_temp_.GetValue();
    Float32 engineLoad = engine_load_.GetValue();
    Float32 outputFanDuty;
    Int32 outputOverHeatFlag;

    wlCoolingFanControl(&coolantTemp, &engineLoad, &outputFanDuty, &outputOverHeatFlag);

    fan_duty_output_ = signals::SpeedSignal(outputFanDuty, signals::SignalValidity::VALID);

    over_heat_flag_output_ = signals::BoolSignal(outputOverHeatFlag == 1, signals::SignalValidity::VALID);
 }

 const signals::PercentageSignal& WlCoolingFanControl::FanDutyRef() const {
    return fan_duty_output_;
 }
 const signals::BoolSignal& WlCoolingFanControl::OverHeatFlagRef() const {
    return over_heat_flag_output_;
 }

 