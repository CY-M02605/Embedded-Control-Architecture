/**
 * @file fan_cooling_control.h
 * @brief declare FanCoolingControl module for OILTEMPWARNINGDEMO project
 * @date 2026-06-16
 */

#ifndef FAN_COOLING_CONTROL_h
#define FAN_COOLING_CONTROL_h

#include "../framework/manager.h"
#include "../framework/module_interface.h"

#include "../signals/signal.h"

#include "../utility/hysteresis.h"

namespace modules {
class FanCoolingControl: public framework::ModuleInterface {
    public:
        struct Config {
            float low_cooling_temp;
            float high_cooling_temp;
        };

        explicit FanCoolingControl(
            Config config,
            const signals::FloatSignal& oil_temp,
            framework::Manager& manager
        );

        void Update() override;

        const signals::BoolSignal& FanCoolingRef() const;

    private:
        const Config config_;
        const signals::FloatSignal& oil_temp_;
        signals::BoolSignal fan_cooling_output_;

        utility::Hysteresis hysteresis_;
};
}

#endif
