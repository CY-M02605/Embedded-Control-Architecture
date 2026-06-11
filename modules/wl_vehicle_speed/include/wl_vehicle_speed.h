 /**
  * @file wl_vehicle_speed.h
  * @brife WlVehicleSpeed for the wl_vehicle_speed
  * @date 01.06.2026
  */

  #ifndef WL_VEHICLE_SPEED
  #define WL_VEHICLE_SPEED

  #include "manager.h"
  #include "module_interface.h"

  #include "signals/signal.h"

  namespace wl_vehicle_speed {
  class WlVehicleSpeed: public framework::ModuleInterface {
    public:
        WlVehicleSpeed (
            const signals::SpeedSignal& engine_rotation,
            const signals::ShiftSignal& shift_signal,
            framework::Manager& manager);

        void Init();
        void Update() override;

        const signals::SpeedSignal& VehicleSpeedRef() const;

    private:
        const signals::SpeedSignal& engine_rotation_;
        const signals::ShiftSignal& shift_signal_;
        signals::SpeedSignal vehicle_Speed_output_;
  };
  }

  #endif
