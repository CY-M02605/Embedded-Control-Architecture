#ifndef SPEED_MONITOR_H
#define SPEED_MONITOR_H

#include "signal.h"
#include "module_interface.h"
#include "manager.h"

namespace modules {

// Monitors vehicle speed and sets over-speed flag.
// Constructor:  (const SpeedSignal& speed,
//                const signals::Signal<float>& limit,
//                Manager& manager)
// Output:       OverSpeedRef() -> BoolSignal (true = over speed limit)
class SpeedMonitor : public framework::ModuleInterface {
public:
    SpeedMonitor(
        const signals::FloatSignal& speed,
        const signals::FloatSignal& limit,
        framework::Manager& manager)
        : speed_(speed), 
        limit_(limit), 
        over_speed_(false, signals::ValidityStatus::VALID)
    {
        manager.RegisterModule(*this);
    }

    void Update() override {
        bool over = speed_.IsValid() && (speed_.GetValue() > limit_.GetValue());
        over_speed_ = signals::BoolSignal(over, signals::ValidityStatus::VALID);
    }

    const signals::BoolSignal& OverSpeedRef() const { return over_speed_; }

private:
    const signals::FloatSignal& speed_;
    const signals::FloatSignal& limit_;
    signals::BoolSignal over_speed_;
};

}  // namespace modules

#endif
