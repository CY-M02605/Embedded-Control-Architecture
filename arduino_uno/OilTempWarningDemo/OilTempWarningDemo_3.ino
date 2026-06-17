#include "src/framework/manager.h"
#include "src/signals/signal.h"
#include "src/modules/oil_temp_warning.h"
#include "src/modules/fan_cooling_control.h"
#include "src/utility/increment_timer.h"

// Hardware configuration
constexpr int OIL_TEMP_INPUT_PIN = A0;

constexpr int WARNING_LED_PIN = LED_BUILTIN;   // D13
constexpr int FAN_COOLING_PIN = 8;

// Demo configuration
constexpr float MAX_SIMULATED_TEMP_C = 120.0f;
constexpr unsigned long CONTROL_INTERVAL_MS = 100UL;

constexpr float WARNING_LOW_THRESHOLD = 90.0f;
constexpr float WARNING_HIGH_THRESHOLD = 95.0f;

constexpr float COOLING_LOW_THRESHOLD = 80.0f;
constexpr float COOLING_HIGH_THRESHOLD = 85.0f;

// Framework object
framework::Manager manager;

// Shared input signal
signals::FloatSignal oil_temp_signal(
    0.0f,
    signals::ValidityStatus::INVALID
);

// Module configuration
const modules::OilTempWarning::Config oil_temp_warning_config{
    WARNING_LOW_THRESHOLD,
    WARNING_HIGH_THRESHOLD
};

const modules::FanCoolingControl::Config fan_cooling_config {
    COOLING_LOW_THRESHOLD,
    COOLING_HIGH_THRESHOLD
};

// Module object
modules::OilTempWarning oil_temp_warning(
    oil_temp_warning_config,
    oil_temp_signal,
    manager
);

modules::FanCoolingControl fan_cooling_warning(
    fan_cooling_config,
    oil_temp_signal,
    manager
);

// Non-blocking control timer
utility::IncrementTimer control_timer(
    CONTROL_INTERVAL_MS
);

void setup() {
    Serial.begin(9600);

    pinMode(OIL_TEMP_INPUT_PIN, INPUT);
    pinMode(WARNING_LED_PIN, OUTPUT);
    pinMode(FAN_COOLING_PIN, OUTPUT);

    digitalWrite(WARNING_LED_PIN, LOW);
    digitalWrite(FAN_COOLING_PIN, LOW);

    Serial.println("OilTempWarningDemo started.");
}

void loop() {
    if (!control_timer.IsExpired()) {
        return;
    }

    // Read potentiometer or analog voltage from A0.
    const int raw_adc = analogRead(OIL_TEMP_INPUT_PIN);

    // Convert ADC range 0-1023 to simulated temperature 0-120 C.
    const float oil_temp_c =
        static_cast<float>(raw_adc)
        * MAX_SIMULATED_TEMP_C
        / 1023.0f;

    // Update the shared input signal.
    oil_temp_signal.Set(
        oil_temp_c,
        signals::ValidityStatus::VALID
    );

    // Run all registered control modules.
    manager.UpdateAll();

    // Read the OilTempWarning output signal.
    const signals::BoolSignal& lamp_warning_signal =
        oil_temp_warning.WarningSignalRef();

    const bool warning_active =
        lamp_warning_signal.IsValid()
        && lamp_warning_signal.GetValue();

    // Read the FanCooling output signal.
    const signals::BoolSignal& fan_cooling_signal = 
        fan_cooling_warning.FanCoolingRef();

    const bool cooling_active = 
        fan_cooling_signal.IsValid()
        && fan_cooling_signal.GetValue();

    // Drive the built-in LED.
    digitalWrite(
        WARNING_LED_PIN,
        warning_active ? HIGH : LOW
    );

    digitalWrite(
        FAN_COOLING_PIN,
        cooling_active ? HIGH : LOW
    );

    // Debug output.
    Serial.print("ADC: ");
    Serial.print(raw_adc);

    Serial.print(" | Oil temp: ");
    Serial.print(oil_temp_c);
    Serial.print(" C");

    Serial.print(" | Warning: ");
    Serial.print(warning_active ? "ON" : "OFF");

    Serial.print(" | fan_cooling: ");
    Serial.println(cooling_active ? "ON" : "OFF");
}