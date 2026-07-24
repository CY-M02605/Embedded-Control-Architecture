#include "hydraulic_oil_warning.h"

using modules::HydraulicOilWarningFunction;

HydraulicOilWarningFunction::HydraulicOilWarningFunction(
        const modules::HydraulicOilWarningFunction::Config& config,
        const signals::FloatSignal& oil_temp,
        const signals::BoolSignal& is_diag_normal,
        framework::Manager& manager
):config_(config),
oil_temp_high_(config.HysteresisConfig),
display_timer_(config.IncrementTimerConfig),
oil_temp_(oil_temp),
is_diag_normal_(is_diag_normal),
warning_lamp_(signals::OnOffStatus::OFF, signals::ValidityStatus::VALID),
status_(STATE_OFF){
    manager.RegisterModule(*this);
}

void HydraulicOilWarningFunction::Update() {
    
    oil_temp_high_.Update(oil_temp_.GetValue());

    switch (status_)
    {
    case STATE_OFF:
        if (oil_temp_high_.GetState() == utility::Hysteresis::ON && is_diag_normal_.GetValue()) {
            status_ = STATE_ON;
            display_timer_.Clear();
            warning_lamp_.Set(signals::OnOffStatus::ON, signals::ValidityStatus::VALID);
        } else {
            warning_lamp_.Set(signals::OnOffStatus::OFF, signals::ValidityStatus::VALID);
        }
        break;
    
    case STATE_ON:
        display_timer_.Update();
        if (!is_diag_normal_.GetValue() 
            || (oil_temp_high_.GetState() != utility::Hysteresis::ON && display_timer_.IsTimeUp())) {
            status_ = STATE_OFF;
            warning_lamp_.Set(signals::OnOffStatus::OFF, signals::ValidityStatus::VALID);
        } else {
            warning_lamp_.Set(signals::OnOffStatus::ON, signals::ValidityStatus::VALID);
        }
        break;
    }
}

const signals::OnOffSignal& HydraulicOilWarningFunction::WarningLamp() const{
    return warning_lamp_;
}
