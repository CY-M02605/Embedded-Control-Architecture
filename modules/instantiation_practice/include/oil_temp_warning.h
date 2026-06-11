#ifndef OIL_TEMP_WARNING_H
#define OIL_TEMP_WARNING_H

#include "signal.h"
#include "module_interface.h"
#include "manager.h"

namespace modules {

// Warns when oil temperature is too high.
// Constructor:  (const TemperatureSignal& oil_temp, Manager& manager)
// Output:       WarnSignalRef() -> BoolSignal (true = warning ON)
class OilTempWarning : public framework::ModuleInterface {
public:
    OilTempWarning(
        const signals::FloatSignal& oil_temp,
        framework::Manager& manager)
        : oil_temp_(oil_temp), 
        warn_output_(false, signals::ValidityStatus::VALID)
    {
        manager.RegisterModule(*this);
    }

    void Update() override {
        bool warn = oil_temp_.IsValid() && (oil_temp_.GetValue() >= 95.0f);
        warn_output_ = signals::BoolSignal(warn, signals::ValidityStatus::VALID);
    }

    const signals::BoolSignal& WarnSignalRef() const { return warn_output_; }

private:
    const signals::FloatSignal& oil_temp_;
    signals::BoolSignal warn_output_;
};

}  // namespace modules

#endif
