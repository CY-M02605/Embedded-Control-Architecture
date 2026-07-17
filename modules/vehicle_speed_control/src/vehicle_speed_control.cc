/**
 * @file wl_vehicle_speed.cc
 * @brife WlVehicleSpeed for the wl_vehicle_speed
 * @date 01.06.2026
 */

 #include "vehicle_speed_control.h"

 extern "C" {
 #include "vehicleSpeedControl.h"
 }

 using vehicle_speed_control::VehicleSpeedControl;

 VehicleSpeedControl::VehicleSpeedControl(
    const signals::FloatSignal& engine_rotation,
    const signals::IntSignal& shift_signal,
    framework::Manager& manager
 ):engine_rotation_(engine_rotation),
 shift_signal_(shift_signal),
 vehicle_Speed_output_(0.0f, signals::ValidityStatus::VALID) {
    manager.RegisterModule(*this);
 }

 void VehicleSpeedControl::Init() {
    INIT_vehicleSpeedControl();
 }

 void VehicleSpeedControl::Update() {
    Float32 engineRpm = engine_rotation_.GetValue();
    Int32 gearRatio = shift_signal_.GetValue();

    float vehicleSpeed;

    vehicleSpeedContol(&engineRpm, &gearRatio, &vehicleSpeed);
    
    vehicle_Speed_output_ = signals::FloatSignal(vehicleSpeed, signals::ValidityStatus::VALID);

 }

 const signals::FloatSignal& VehicleSpeedControl::VehicleSpeedRef() const {
    return vehicle_Speed_output_;
 }

