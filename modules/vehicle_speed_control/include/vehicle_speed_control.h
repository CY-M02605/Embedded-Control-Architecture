 /**
  * @file vehicle_speed_control.h
  * @brife vehicleSpeedControl for the vehicle_speed_control
  * @date 01.06.2026
  */

  #ifndef VEHICLE_SPEED_CONTROL_H
  #define VEHICLE_SPEED_CONTROL_H

  #include "manager.h"
  #include "module_interface.h"

  #include "signal.h"

  namespace vehicle_speed_control {
  class VehicleSpeedControl: public framework::ModuleInterface {
    public:
        VehicleSpeedControl (
            const signals::FloatSignal& engine_rotation,
            const signals::IntSignal& shift_signal,
            framework::Manager& manager);

        void Init();
        void Update() override;

        const signals::FloatSignal& VehicleSpeedRef() const;

    private:
        const signals::FloatSignal& engine_rotation_;
        const signals::IntSignal& shift_signal_;
        signals::FloatSignal vehicle_Speed_output_;
  };
  }

  #endif
