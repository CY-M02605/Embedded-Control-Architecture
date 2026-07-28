#include "src/framework/manager.h"
#include "src/framework/module_interface.h"
#include "src/signals/signal.h"
#include "src/utility/increment_timer.h"
#include "src/utility/lookup_table_1d.h"

#include <Arduino.h>

// Hardware configuration
constexpr uint8_t OIL_TEMP_INPUT_PIN = A0;
constexpr uint8_t IS_ENGINE_RUNNING_INPUT_PIN = A1; 

constexpr uint8_t IS_OVERHEAT_PROTECTION_OUTPUT_PIN = 8;
constexpr uint8_t TORQUE_LIMIT_OUTPUT_PIN = 9;
constexpr uint8_t FAN_REQUEST_OUTPUT_PIN = 10;

// Demo configuration

// Framework object

// Shared input signal

// Module configuration

// Module object

// Non-blocking control timer

void Setup() {

}

void loop() {

}