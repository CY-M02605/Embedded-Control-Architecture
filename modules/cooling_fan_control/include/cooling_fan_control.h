/**
 * @file cooling_fan_control.h
 * @brief CoolingFanControl for the cooling_fan_control
 * @date 02.06.2026
 */

 #ifndef COOLING_FAN_CONTROL_H
 #define COOLING_FAN_CONTROL_H

 #include "module_interface.h"
 #include "signals/signal.h"
 #include "framework/manager.h"

 namespace cooling_fan_control {
 class CoolingFanControl: public framework::ModuleInterface {
    public:
        CoolingFanControl(
            const signals::FloatSignal& coolant_temp,
            const signals::FloatSignal& engine_load,
            framework::Manager& manager
        );

        void Init();
        void Update() override;

        const signals::FloatSignal& FanDutyRef() const;
        const signals::BoolSignal& OverHeatFlagRef() const;

    private:
        const signals::FloatSignal& coolant_temp_;
        const signals::FloatSignal& engine_load_;
        signals::FloatSignal fan_duty_output_;
        signals::BoolSignal over_heat_flag_output_;

 };
 }



 #endif
