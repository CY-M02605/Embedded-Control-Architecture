/**
 * @file oil_temp_warning.h
 * @brief Declares the OilTempWarning module for Arduino UNO.
 * @date 2026-06-15
 */

#ifndef OIL_TEMP_WARNING
#define OIL_TEMP_WARNING

#include "../framework/manager.h"
#include "../framework/module_interface.h"
#include "../signals/signal.h"

#include "../utility/hysteresis.h"
// #include "../utility/increment_timer.h"

namespace modules {
class OilTempWarning: public framework::ModuleInterface {
    public:
        struct Config {
            utility::Hysteresis::Config hysteresis_config;
        };

        explicit OilTempWarning(
            modules::OilTempWarning::Config config,
            const signals::FloatSignal& oil_temp,
            framework::Manager& manager
        );

        void Update() override;

        const signals::BoolSignal& WarningSignalRef() const;

    private:
        const modules::OilTempWarning::Config config_;
        const signals::FloatSignal& oil_temp_;
        signals::BoolSignal warning_output_;

        utility::Hysteresis hysteresis_;

};
}

#endif
