/**
 * @file EngineOverheatProtection.ino
 * @brief Arduino hardware implementation for engine overheat protection module
 * @date 2026-08-26
 */

#include <Arduino.h>

#include "src/framework/manager.h"
#include "src/signals/signal.h"
#include "src/utility/lookup_table_1d.h"
#include "src/modules/engine_overheat_protection.h"


// Step1: create minimal Arduino system
constexpr uint8_t OIL_TEMP_INPUT_PIN = A0;
constexpr uint8_t ENGINE_RUNNING_INPUT_PIN = 2;
constexpr uint8_t CLEAR_FAULT_REQUEST_INPUT_PIN = 3;

framework::Manager manager;

signals::FloatSignal oil_temp_input(0.0f, signals::ValidityStatus::VALID);
signals::BoolSignal is_engine_running_input(false, signals::ValidityStatus::VALID);
signals::BoolSignal clear_fault_request_input(false, signals::ValidityStatus::VALID);

using Table = utility::LookupTable1D<float>;

// Step2-1: define lookup table
const Table::Points oil_temp_torque_limit_table[] = {
    {80.0f, 100.0f},
    {90.0f, 100.0f},
    {100.0f, 100.0f},
    {105.0f, 95.0f},
    {110.0f, 80.0f},
    {115.0f, 60.0f},
    {120.0f, 30.0f}
};

const Table::Points oil_temp_fan_request_table[] = {
    {80.0f, 20.0f},
    {90.0f, 40.0f},
    {100.0f, 65.0f},
    {105.0f, 80.0f},
    {110.0f, 100.0f},
    {115.0f, 100.0f},
    {120.0f, 100.0f}
};

// Step2-2: complete the configuration
engine_overheat_protection::EngineOverheatProtection::Config config {
    90.0f,
    80.0f,
    125.0f,
    -25.0f,
    15.0f,
    {3},
    oil_temp_torque_limit_table,
    sizeof(oil_temp_torque_limit_table) / sizeof(oil_temp_torque_limit_table[0]),
    oil_temp_fan_request_table,
    sizeof(oil_temp_fan_request_table) / sizeof(oil_temp_fan_request_table[0])
};

// Step3: create global module object
engine_overheat_protection::EngineOverheatProtection engine_overheat_protection_module (
    config,
    oil_temp_input,
    is_engine_running_input,
    clear_fault_request_input,
    manager
);

// Step4: Initial setup()
void setup() {
    Serial.begin(9600);

    pinMode(OIL_TEMP_INPUT_PIN, INPUT);
    pinMode(ENGINE_RUNNING_INPUT_PIN, INPUT_PULLUP);
    pinMode(CLEAR_FAULT_REQUEST_INPUT_PIN, INPUT_PULLUP);

    Serial.println("Engine overheat protection module demo starts.");
}

// Step5: Set inputs, outputs and ptintln() in loop()
void loop() {
    int raw_oil_temp_input = analogRead(OIL_TEMP_INPUT_PIN);

    float oil_temp_value = AnalogValueToOilTempMap(raw_oil_temp_input);

    bool is_engine_running_value = digitalRead(ENGINE_RUNNING_INPUT_PIN) == LOW;

    bool clear_fault_request_value = digitalRead(CLEAR_FAULT_REQUEST_INPUT_PIN) == LOW;

    oil_temp_input.Set(oil_temp_value, signals::ValidityStatus::VALID);
    is_engine_running_input.Set(is_engine_running_value, signals::ValidityStatus::VALID);
    clear_fault_request_input.Set(clear_fault_request_value, signals::ValidityStatus::VALID);

    manager.UpdateAll();

    Serial.print("oil temp: ");
    Serial.print(oil_temp_value);

    Serial.print("| is engine running: ");
    Serial.print(is_engine_running_value);

    Serial.print("| clear fault request: ");
    Serial.print(clear_fault_request_value);

    Serial.print("| state: ");
    Serial.print(StateToStr(engine_overheat_protection_module.StateRef()));

    Serial.print("| fault reason: ");
    Serial.print(FaultReasonToStr(engine_overheat_protection_module.FaultReasonRef()));

    Serial.print("| engine overheat protection status: ");
    Serial.print(engine_overheat_protection_module.IsOverheatProtectedRef().GetValue());

    Serial.print("| torque limit percent: ");
    Serial.print(engine_overheat_protection_module.TorqueLimitRef().GetValue());

    Serial.print("| fan request percent: ");
    Serial.println(engine_overheat_protection_module.FanRequestRef().GetValue());

    delay(1000);
}

float AnalogValueToOilTempMap(int raw_value) {
    return -30.0f + static_cast<float>(raw_value) * 160.0f / 1023.0f;
}

const char* StateToStr(engine_overheat_protection::EngineOverheatProtectionState raw_state) {
    switch (raw_state)
    {
    case engine_overheat_protection::EngineOverheatProtectionState::STOP:
        return "STOP";

    case engine_overheat_protection::EngineOverheatProtectionState::IDLE:
        return "IDLE";

    case engine_overheat_protection::EngineOverheatProtectionState::COUNTING:
        return "COUNTING";

    case engine_overheat_protection::EngineOverheatProtectionState::PROTECTED:
        return "PROTECTED";

    case engine_overheat_protection::EngineOverheatProtectionState::AFTER_RUN_COOLING:
        return "AFTER_RUN_COOLING";

    case engine_overheat_protection::EngineOverheatProtectionState::FAULT:
        return "FAULT";
    }

    return "error";
}

const char* FaultReasonToStr(engine_overheat_protection::FaultReason raw_fault_reason) {
    switch (raw_fault_reason)
    {
    case engine_overheat_protection::FaultReason::NONE:
        return "NONE";

    case engine_overheat_protection::FaultReason::OIL_TEMP_SIGNAL_INVALID:
        return "OIL_TEMP_SIGNAL_INVALID";

    case engine_overheat_protection::FaultReason::ENGINE_RUNNING_SIGNAL_INVALID:
        return "ENGINE_RUNNING_SIGNAL_INVALID";

    case engine_overheat_protection::FaultReason::UNEXPECTED_HIGH_TEMP_IN_STOP:
        return "UNEXPECTED_HIGH_TEMP_IN_STOP";

    case engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_HIGH:
        return "TEMP_OUT_OF_RANGE_HIGH";

    case engine_overheat_protection::FaultReason::TEMP_OUT_OF_RANGE_LOW:
        return "TEMP_OUT_OF_RANGE_LOW"
    }

    return "error";
}