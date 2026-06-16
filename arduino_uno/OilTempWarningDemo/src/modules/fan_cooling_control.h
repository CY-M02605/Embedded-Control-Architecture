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

namespace modules {
class FanCoolingControl: public framework::ModuleInterface {
    public:
        struct Config {
            float cooling_temp;
        };
    private:
};
}

#endif
