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
constexpr uint_8 OIL_TEMP_INPUT_PIN = A0;
constexpr unit_8 ENGINE_RUNNING_INPUT_PIN = 2;
constexpr unit_8 CLEAR_FAULT_REQUEST_INPUT_PIN = 3;

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
    pinMode(clear_fault_request_input, INPUT_PULLUP);

    Serial.println("Engine overheat protection module demo starts.");
}

// Step5: Set inputs, outputs and ptintln() in loop()
void loop() {
    int raw_oil_temp_input = analogRead(OIL_TEMP_INPUT_PIN);

    float oil_temp_value = AnalogValueToOilTempMap(raw_oil_temp_input);

    bool is_engine_running_value = digitalRead(ENGINE_RUNNING_INPUT_PIN) == LOW;

    bool clear_fault_request_value = digitalRead(CLEAR_FAULT_REQUEST_INPUT_PIN) == LOW;

    oil_temp_input(oil_temp_value, signals::ValidityStatus::VALID);
    is_engine_running_input(is_engine_running_value, signals::ValidityStatus::VALID);
    clear_fault_request_input(clear_fault_request_value, signals::ValidityStatus::VALID);

    manager.UpdateAll();

    Serial.println("oil temp: ");
    Serial.println(oil_temp_value);

    Serial.println("| is engine running: ");
    Serial.println(is_engine_running_value);

    Serial.println("| clear fault request: ");
    Serial.println(clear_fault_request_value);

    Serial.println("| state: ");
    Serial.println(engine_overheat_protection_module.StateRef());

    Serial.println("| fault reason: ");
    Serial.println(engine_overheat_protection_module.FaultReasonRef());

    Serial.println("| engine overheat protection status: ");
    Serial.println(engine_overheat_protection_module.IsOverheatProtectedRef());

    Serial.println("| torque limit percent: ");
    Serial.println(engine_overheat_protection_module.TorqueLimitRef());

    Serial.println("| fan request percent: ");
    Serial.println(engine_overheat_protection_module.FanRequestRef());

    delay(100);
}

float AnalogValueToOilTempMap(int raw_value) {
    return -30.0f + static_cast<float>(raw_value) * 160.0f / 1023.0f;
}