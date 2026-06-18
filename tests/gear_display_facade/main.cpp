/**
 * @file main.cpp
 * @brief test file for gear_display_facade 
 * @date 04.06.2026
 */

#include <iostream>

#include "framework/manager.h"
#include "framework/module_interface.h"

#include "signals/signal.h"
#include "gear_display_facade.h"

#include "gear_types.h"

const char* GearPositionToStr (GearPosition gear) {
    switch (gear) {
        case NEUTRAL:   return "NEUTRAL";   
        case FORWARD_1: return "FORWARD_1"; 
        case FORWARD_2: return "FORWARD_2"; 
        case FORWARD_3: return "FORWARD_3"; 
        case REVERSE_1: return "REVERSE_1"; 
        case REVERSE_2: return "REVERSE_2"; 
        default:        return "UNKNOWN";   
    }
}

const char* DriveModeToStr (DriveMode mode) {
    switch (mode) {
        case ECO:   return "ECO";
        case POWER: return "POWER";
        default:    return "UNKNOWN";
    }
}

const char* ValidityToStr (signals::ValidityStatus validation) {
    return (validation == signals::ValidityStatus::VALID? "VALID" : "INVALID"); 
}

void CheckGearPosition(
    const char* test_name,
    const gear_display_facade::GearPositionSignal& actual,
    GearPosition expected_value,
    signals::ValidityStatus expected_validity) {
        bool result = (actual.GetValue() == expected_value 
                    && actual.GetValidity() == expected_validity);
        const char* pass = result? "PASS" : "FAIL";

    std::cout << "test_name:" << test_name 
              << "| actual_value: " << GearPositionToStr(actual.GetValue())
              << "| expected_value: " << GearPositionToStr(expected_value)
              << "| actual_validity: " << ValidityToStr(actual.GetValidity())
              << "| expected_validity: " << ValidityToStr(expected_validity)
              << "| PASS or FAIL ? " << pass
              << std::endl;
}

void CheckDriveMode(
    const char* test_name,
    const gear_display_facade::DriveModeSignal& actual,
    DriveMode expected_value,
    signals::ValidityStatus expected_validity) {
        bool result = (actual.GetValue() == expected_value 
                    && actual.GetValidity() == expected_validity);
        const char* pass = result? "PASS" : "FAIL";
    
    std::cout << "test_name:" << test_name 
              << "| actual_value: " << DriveModeToStr(actual.GetValue())
              << "| expected_value: " << DriveModeToStr(expected_value)
              << "| actual_validity: " << ValidityToStr(actual.GetValidity())
              << "| expected_validity: " << ValidityToStr(expected_validity)
              << "| PASS : FAIL ? " << pass
              << std::endl;
}

int main() {

    std::cout << "============ Gear Display Facade Test ============" << std::endl;

    // 1、Create the manager
    framework::Manager manager;

    // 2、Create raw input signals
    gear_display_facade::AcGearPositionSignal ac_gear_position_signal(0, signals::ValidityStatus::VALID);
    gear_display_facade::AcIsEcoModeSignal ac_is_eco_mode_signal(false, signals::ValidityStatus::VALID);

    // 3、Create GearDisplayFacade
    gear_display_facade::GearDisplayFacade Facade(
        ac_gear_position_signal,
        ac_is_eco_mode_signal,
        manager
    ); 

    // 4、Set raw input signal
    struct GearInputs {
        int raw_gear;
        signals::ValidityStatus input_validity;
        GearPosition expected_position;
        signals::ValidityStatus expected_validation;
    };

    GearInputs GearInputsArray[] = {
        {0, signals::ValidityStatus::VALID, NEUTRAL, signals::ValidityStatus::VALID}, 
        {1, signals::ValidityStatus::VALID, FORWARD_1, signals::ValidityStatus::VALID},
        {2, signals::ValidityStatus::VALID, FORWARD_2, signals::ValidityStatus::VALID},
        {3, signals::ValidityStatus::VALID, FORWARD_3, signals::ValidityStatus::VALID},
        {-1, signals::ValidityStatus::VALID, REVERSE_1, signals::ValidityStatus::VALID},
        {-2, signals::ValidityStatus::VALID, REVERSE_2, signals::ValidityStatus::VALID},
        {99, signals::ValidityStatus::VALID, NEUTRAL, signals::ValidityStatus::VALID},
        {0, signals::ValidityStatus::INVALID, NEUTRAL, signals::ValidityStatus::INVALID}, 
        {1, signals::ValidityStatus::INVALID, NEUTRAL, signals::ValidityStatus::INVALID},
        {2, signals::ValidityStatus::INVALID, NEUTRAL, signals::ValidityStatus::INVALID},
        {3, signals::ValidityStatus::INVALID, NEUTRAL, signals::ValidityStatus::INVALID},
        {-1, signals::ValidityStatus::INVALID, NEUTRAL, signals::ValidityStatus::INVALID},
        {-2, signals::ValidityStatus::INVALID, NEUTRAL, signals::ValidityStatus::INVALID},
        {99, signals::ValidityStatus::INVALID, NEUTRAL, signals::ValidityStatus::INVALID}
    };
    
    int gearArrayLength = sizeof(GearInputsArray) / sizeof(GearInputs);

    std::cout << "------------- Gear Position Test ------------" << std::endl;

    for (int i = 0; i < gearArrayLength; ++i) {
        ac_gear_position_signal.Set(
            GearInputsArray[i].raw_gear, 
            GearInputsArray[i].input_validity);
        std::cout << "[Gear Test " << (i + 1) << "] ";
        manager.UpdateAll();
        CheckGearPosition(
            "Gear test",
            Facade.GearPositionRef(),
            GearInputsArray[i].expected_position,
            GearInputsArray[i].expected_validation
        );
    }

    struct ModeInputs {
        bool raw_mode;
        signals::ValidityStatus input_validity;
        DriveMode expected_mode;
        signals::ValidityStatus expected_validation;
    };

    ModeInputs ModeInputsArray[] = {
        {false, signals::ValidityStatus::VALID, POWER, signals::ValidityStatus::VALID}, 
        {true, signals::ValidityStatus::VALID, ECO, signals::ValidityStatus::VALID},
        {false, signals::ValidityStatus::INVALID, POWER, signals::ValidityStatus::INVALID}, 
        {true, signals::ValidityStatus::INVALID, POWER, signals::ValidityStatus::INVALID},
    };

    int modeArrayLength = sizeof(ModeInputsArray) / sizeof(ModeInputs);

    std::cout << "------------- Drive Mode Test ------------" << std::endl;

    ac_gear_position_signal.Set(0, signals::ValidityStatus::VALID);

    for (int j = 0; j < modeArrayLength; ++j) {
        ac_is_eco_mode_signal.Set(
            ModeInputsArray[j].raw_mode, 
            ModeInputsArray[j].input_validity);
        manager.UpdateAll();
        std::cout << "[Mode Test " << j + 1 << "]";
        CheckDriveMode(
            "Mdoe test",
            Facade.DriveModeRef(),
            ModeInputsArray[j].expected_mode,
            ModeInputsArray[j].expected_validation
        );
    }
    
    return 0;

}
