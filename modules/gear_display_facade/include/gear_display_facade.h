#ifndef GEAR_DISPLAY_FACADE_H
#define GEAR_DISPLAY_FACADE_H

/**
 * @file gear_display_facade.h
 * @brife GearDisplayFacade for gear_display_facade
 * @date 04.06.2026
 */

// Do not get any spaces in front of #include
#include "framework/manager.h"
#include "framework/module_interface.h"
#include "signals/signal.h"
#include "gear_types.h"

 namespace gear_display_facade {

 typedef signals::Signal <int> AcGearPositionSignal;
 typedef signals::BoolSignal AcIsEcoModeSignal;
 
 typedef signals::Signal <GearPosition> GearPositionSignal;
 typedef signals::Signal <DriveMode> DriveModeSignal;


 class GearDisplayFacade: public framework::ModuleInterface {
    public:
        GearDisplayFacade(
            const AcGearPositionSignal& ac_gear_position_signal,
            const AcIsEcoModeSignal& ac_is_eco_mode_signal,
            framework::Manager& manager
        );

        void Update() override;

        const GearPositionSignal& GearPositionRef() const;
        const DriveModeSignal& DriveModeRef() const;
        
    private:
        void UpdateGearPosition();
        void UpdateDriveMode();

        const AcGearPositionSignal& ac_gear_position_signal_;
        const AcIsEcoModeSignal& ac_is_eco_mode_signal_;

        GearPositionSignal gear_position_output_;
        DriveModeSignal drive_mode_output_;
        
 };


 }


 #endif
  