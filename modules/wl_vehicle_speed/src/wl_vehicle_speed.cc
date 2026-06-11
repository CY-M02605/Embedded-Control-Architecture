/**
 * @file wl_vehicle_speed.cc
 * @brife WlVehicleSpeed for the wl_vehicle_speed
 * @date 01.06.2026
 */

 #include "wl_vehicle_speed.h"

 extern "C" {
 #include "wlVehicleSpeed.h"
 }

 using wl_vehicle_speed::WlVehicleSpeed;

 WlVehicleSpeed::WlVehicleSpeed(
    const signals::SpeedSignal& engine_rotation,
    const signals::ShiftSignal& shift_signal,
    framework::Manager& manager
 ):engine_rotation_(engine_rotation),
 shift_signal_(shift_signal),
 vehicle_Speed_output_(0.0f, signals::SignalValidity::VALID) {
    manager.RegisterModule(*this);
 }

 void WlVehicleSpeed::Init() {
    INIT_wlVehicleSpeed();
 }

 void WlVehicleSpeed::Update() {
    Float32 engineRpm = engine_rotation_.GetValue();
    Int32 gearRatio = shift_signal_.GetValue();

    // 不需要从旧输入取值
    // Float32 vehicleSpeed = vehicle_Speed_output_.GetValue();

    // 不需要初始化，C函数会赋值
    Float32 vehicleSpeed;

    wlVehicleSpeed(&engineRpm, &gearRatio, &vehicleSpeed);
    
    vehicle_Speed_output_ = signals::SpeedSignal(vehicleSpeed, signals::SignalValidity::VALID);

 }

 const signals::SpeedSignal& WlVehicleSpeed::VehicleSpeedRef() const {
    return vehicle_Speed_output_;
 }

