/**
 * @file gear_display_facade.c
 * @brief a implementation of gear_display_facade.h
 * @date 04.06.2026
 */

#include "gear_display_facade.h"

namespace gear_display_facade {

    GearDisplayFacade::GearDisplayFacade (
        const AcGearPositionSignal& ac_gear_position_signal,
        const AcIsEcoModeSignal& ac_is_eco_mode_signal,
        framework::Manager& manager
    ):ac_gear_position_signal_(ac_gear_position_signal),
    ac_is_eco_mode_signal_(ac_is_eco_mode_signal),
    gear_position_output_(NEUTRAL, signals::ValidityStatus::VALID),
    drive_mode_output_(POWER, signals::ValidityStatus::VALID){
    manager.RegisterModule(*this);
    }

    void GearDisplayFacade::Update() {
        UpdateGearPosition();
        UpdateDriveMode();
    }
    
    const GearPositionSignal& GearDisplayFacade::GearPositionRef() const {
        return gear_position_output_;
    }

    const DriveModeSignal& GearDisplayFacade::DriveModeRef() const {
        return drive_mode_output_;
    }

    void GearDisplayFacade::UpdateGearPosition() {
        if (ac_gear_position_signal_.GetValidity() != signals::ValidityStatus::VALID) {
            gear_position_output_.Set(NEUTRAL, signals::ValidityStatus::INVALID);
            return;
        }

        switch (ac_gear_position_signal_.GetValue())
        {
        case 0:
            gear_position_output_.SetValue(NEUTRAL);
            break;
        
        case 1:
            gear_position_output_.SetValue(FORWARD_1);
            break;

        case 2:
            gear_position_output_.SetValue(FORWARD_2);
            break;

        case 3:
            gear_position_output_.SetValue(FORWARD_3);
            break;

        case -1:
            gear_position_output_.SetValue(REVERSE_1);
            break;

        case -2:
            gear_position_output_.SetValue(REVERSE_2);
            break;
            
        default:
            gear_position_output_.SetValue(NEUTRAL);
            break;
        }
    }

    void GearDisplayFacade::UpdateDriveMode() {
        if (ac_is_eco_mode_signal_.GetValidity() != signals::ValidityStatus::VALID) {
            drive_mode_output_.Set(POWER, signals::ValidityStatus::INVALID);
            return;
        }

        drive_mode_output_.SetValue((ac_is_eco_mode_signal_.GetValue() == true)? ECO : POWER); 
    }
}






