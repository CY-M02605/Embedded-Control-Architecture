/**
 * @file wl_cooling_fan_control.h
 * @brife wlCoolingFanControl for the wl_cooling_fan_control
 * @date 02.06.2026
 */

 #ifndef WL_COOLING_FAN_CONTROL_H
 #define WL_COOLING_FAN_CONTROL_H

 #include "module_interface.h"
 #include "signals/signal.h"
 #include "framework/manager.h"

 namespace wl_cooling_fan_control {
 class WlCoolingFanControl: public framework::ModuleInterface {
    public:
        WlCoolingFanControl(
            const signals::TemperatureSignal& coolant_temp,
            const signals::PercentageSignal& engine_load,
            framework::Manager& manager
        );

        void Init();
        void Update() override;

        const signals::SpeedSignal& FanDutyRef() const;
        const signals::BoolSignal& OverHeatFlagRef() const;

    private:
        const signals::TemperatureSignal& coolant_temp_;
        const signals::PercentageSignal& engine_load_;
        signals::PercentageSignal fan_duty_output_;
        signals::BoolSignal over_heat_flag_output_;

 };
 }



 #endif
