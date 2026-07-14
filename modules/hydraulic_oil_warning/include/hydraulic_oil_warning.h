#ifndef HYDRAULIC_OIL_WARNING_H
#define HYDRAULIC_OIL_WARNING_H

// remember the correct include path, the src path has been registered in CMakeLists file
#include "manager.h"
#include "module_interface.h"
#include "signal.h"
#include "hysteresis.h"
#include "increment_timer.h"

namespace modules {
class HydraulicOilWarningFunction: public framework::ModuleInterface {
    public:
        struct Config {
            utility::Hysteresis<float>::Config HysteresisConfig;
            utility::IncrementTimer::Config IncrementTimerConfig;
        };

        enum STATUS {
            STATE_OFF,
            STATE_ON
        };

        HydraulicOilWarningFunction (
            const modules::HydraulicOilWarningFunction::Config& config,
            const signals::FloatSignal& oil_temp,
            const signals::BoolSignal& is_diag_normal,
            framework::Manager& manager
        );

        void Update();

        const signals::OnOffSignal& WarningLamp() const;

    private:
        const modules::HydraulicOilWarningFunction::Config config_;
        utility::Hysteresis<float> oil_temp_high_;
        utility::IncrementTimer display_timer_;
        const signals::FloatSignal& oil_temp_;
        const signals::BoolSignal& is_diag_normal_;
        signals::OnOffSignal warning_lamp_;
        STATUS status_;
};
}

#endif
