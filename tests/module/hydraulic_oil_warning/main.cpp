#include <iostream>

#include "manager.h"
#include "module_interface.h"

#include "signal.h"
#include "hydraulic_oil_warning.h"

const char* OnOffStr(signals::OnOffState s) {
    return (s == signals::ON) ? "ON" : "OFF";
}

int main() {
    framework::Manager manager;

    signals::TemperatureSignal oil_temp(80.0f, signals::VALID);
    signals::BoolSignal is_diag_normal(true, signals::VALID);

    modules::HydraulicOilWarningFunction::Config config;
    config.HysteresisConfig.high_threshold = 105.0f;
    config.HysteresisConfig.low_threshold = 95.0f;
    config.IncrementTimerConfig.threshold = 3;

    modules::HydraulicOilWarningFunction warning(config, oil_temp, is_diag_normal, manager);

    struct CycleInput {
        float temp;
        bool diag;
    };

    CycleInput scenario[] = {
        { 85.7f, true  },
        { 92.2f, true  },
        { 94.6f, true  },
        { 97.3f, true  },
        {101.5f, true  },
        {111.8f, true  },  // should turn ON
        {109.6f, false },  // diag NG -> OFF immediately
        {107.3f, false },
        {105.4f, false },
        {104.2f, false },
        {103.9f, true  },  // diag OK, hysteresis still ON -> ON
        {102.7f, true  },
        {101.4f, true  },
        {100.1f, true  },
        { 99.6f, true  },
        { 97.2f, true  },
        { 98.5f, true  },
        { 97.4f, true  },
        { 96.8f, true  },
        { 95.9f, true  },
        { 94.2f, true  },  // below 95 + timer done -> OFF
    };

    int num_cycles = sizeof(scenario) / sizeof(scenario[0]);

    for (int i = 0; i < num_cycles; ++i) {
        oil_temp.SetValue(scenario[i].temp);
        is_diag_normal.SetValue(scenario[i].diag);
        manager.UpdateAll();

        std::cout << "Cycle " << (i + 1)
                  << ": Temp=" << scenario[i].temp
                  << " Diag=" << (scenario[i].diag ? "OK" : "NG")
                  << " Lamp=" << OnOffStr(warning.WarningLamp().GetValue())
                  << std::endl;
    }

    return 0;
}
