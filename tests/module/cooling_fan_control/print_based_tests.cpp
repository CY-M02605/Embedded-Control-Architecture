#include <iostream>
#include "manager.h"
#include "module_interface.h"
#include "signal.h"

#include "cooling_fan_control.h"

int main() {
    framework::Manager manager;

    signals::FloatSignal coolant_temp;
    signals::FloatSignal engine_load;

    cooling_fan_control::CoolingFanControl fan_control(coolant_temp, engine_load, manager);

    fan_control.Init();

    struct Inputs {
        float temp;
        float load;
    };

    Inputs InputsArray[] {
        {0.0f, 0.0f},
        {10.0f, 10.0f},
        {20.0f, 20.0f},
        {30.0f, 30.0f},
        {40.0f, 40.0f},
        {50.0f, 50.0f},
        {60.0f, 60.0f},
        {70.0f, 70.0f},
        {70.0f, 81.0f},
        {80.0f, 82.0f},
        {90.0f, 83.0f},
        {95.0f, 84.0f},
        {100.0f, 85.0f},
        {110.0f, 90.0f},
        {120.0f, 90.0f},
        {130.0f, 95.0f},
        {140.0f, 98.0f},
    };

    for (int i = 0; i < sizeof(InputsArray)/sizeof(InputsArray[0]); ++i) {
        coolant_temp.SetValue(InputsArray[i].temp);
        engine_load.SetValue(InputsArray[i].load);

        fan_control.Update();

        std::cout << "Temp: " << coolant_temp.GetValue() 
                  << " Load: " << engine_load.GetValue()
                  << " FanDuty: " << fan_control.FanDutyRef().GetValue()
                  << " OverHeatFlag: " << fan_control.OverHeatFlagRef().GetValue()
                  << std::endl;
    }
}
