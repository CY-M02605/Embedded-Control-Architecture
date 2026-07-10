# Embedded Control Architecture

A vehicle-oriented C++ practice project for learning modular embedded control software architecture.

This project explores how embedded control software can be organized into reusable modules, shared signals, utility components, a manager-based update mechanism, and separate PC and Arduino targets.

The project currently includes:

-   PC-based module simulations and manual tests
-   A modular Arduino UNO oil-temperature warning demo
-   Signal validity handling
-   Manager-based module registration and execution
-   Hysteresis-based warning control
-   Non-blocking periodic execution using `millis()`

------------------------------------------------------------------------

## Project Goals

The main goals of this project are:

-   Practice modular C++ design for embedded systems
-   Understand the relationships between modules, signals, framework, utility, and application layers
-   Build control modules with clear input and output signal interfaces
-   Practice header/source separation
-   Practice CMake-based PC builds and print/assert tests
-   Port reusable control logic to Arduino UNO
-   Learn how to connect software architecture with real hardware input and output

------------------------------------------------------------------------

## Project Structure

``` text
Embedded-Control-Architecture/
├── assembly/
|   ├── instantiation.h
|   └── instantiation_practice.h
|
├── docs/
|   ├── circuit-notes.md
|   └── troubleshooting.md
|
├── examples/
|   ├── oil_temp-fan and torque state.md
|   └── 
|
├── framework/
|   ├── manager.h
|   └── module_interface.h
|
├── modules/
|   ├── gear_display_facade/
|   |   ├── include/
|   |   |   ├── gear_display_facade.h
|   |   |   └── gear_types.h
|   |   └── src/
|   |       └── gear_display_facade.cc
|   ├── hydraulic_oil_warning/
|   |   ├── include/
|   |   |   └── hydraulic_oil_warning.h
|   |   └── src/
|   |       └── hydraulic_oil_warning.cc
|   ├── instantiation_practice/
|   |   ├── include/
|   |   |   ├── instantiation_practice.h
|   |   |   └── speed_monitor.h
|   |   └── src/
|   ├── wl_cooling_fan_control/
|   |   ├── include/
|   |   |   ├── wl_cooling_fan_control.h
|   |   |   └── wlCoolingFanControl.h
|   |   └── src/
|   |       ├── wl_cooling_fan_control.cc
|   |       └── wlCoolingFanControl.cc
|   ├── wl_vehicle_speed/
|   |   ├── include/
|   |   |   ├── wl_vehicle_speed.h
|   |   |   └── wlVehicleSpeed.h
|   |   └── src/
|   |       ├── wl_vehicle_speed.cc
|   |       └── wlVehicleSpeed.cc
|   ├── engine_overheat_protection/
|   |   ├── include/
|   |   |   └── engine_overheat_protection.h
|   |   └── src/
|   |       └── engine_overheat_protection.cc
|
├── signals/
|   └── signal.h
|
├── utility/
|   ├── hysteresis.h
|   └── increment_timer.h
|
├── tests/
|   ├── module/
|   |   ├── gear_display_facade/
|   |   |   ├── CMakeLists.txt
|   |   |   └── main.cpp
|   |   ├── hydraulic_oil_warning/
|   |   |   ├── CMakeLists.txt
|   |   |   └── main.cpp
|   |   ├── instantiation_practice/
|   |   |   ├── CMakeLists.txt
|   |   |   └── main.cpp
|   |   ├── wl_cooling_fan_control/
|   |   |   ├── CMakeLists.txt
|   |   |   └── main.cpp
|   |   └── wl_vehicle_speed/
|   |   |   ├── CMakeLists.txt
|   |   |   └── main.cpp
|   |   └── engine_overheat_protection/
|   |   |   ├── CMakeLists.txt
|   |   |   ├── assert_based_tests.cpp
|   |   |   └── print_based_tests.cpp
|   └── unit/
|       ├── CMakeLists.txt
|       ├── test_hysteresis.cpp
|       ├── test_increment_timer.cpp
|       └── test_lookup_table_1d.cpp
|
├── Arduino_project/
|   └── OilTempWarningDemo/
|       ├── OilTempWarningDemo.ino
|       ├── version management/
|       |   ├── 1/
|       |   |   └── OilTempWarningDemo.ino
|       |   ├── 2/
|       |   |   └── OilTempWarningDemo.ino
|       |   ├── 3/
|       |   |   └── OilTempWarningDemo.ino
|       |
|       └── src/
|           ├── framework/
|           |   ├── manager.h
|           |   └── module_interface.h
|           ├── modules/
|           |   ├── oil_temp_warning.h
|           |   ├── oil_temp_warning.cpp
|           |   ├──  fan_cooling_control.h
|           |   ├──  fan_cooling_control.cpp
|           |
|           ├── signals/
|           |   └── signal.h
|           └── utility/
|               ├── hysteresis.h
|               └── increment_timer.h
└── README.md
```

------------------------------------------------------------------------

## Architecture Overview

The project is organized into several logical layers.

### Framework

The `framework` layer provides the basic execution structure.

-   `ModuleInterface` defines a common interface for control modules.
-   `Manager` stores registered modules.
-   `Manager::UpdateAll()` calls the `Update()` function of every registered module.

Example:

``` cpp
class ModuleInterface {
public:
    virtual ~ModuleInterface() = default;
    virtual void Update() = 0;
};
```

This allows different modules to be executed through the same interface.

------------------------------------------------------------------------

### Signals

The `signals` layer provides typed signal objects for passing data between the application layer and control modules.

A signal contains:

-   a value
-   a validity status

The validity status allows a module to distinguish between a real input value and unavailable or invalid data.

The input source can later be changed without modifying the control module. For example, the oil-temperature signal could come from:

-   an Arduino analog input
-   a real temperature sensor
-   CAN communication
-   PC simulation data

------------------------------------------------------------------------

### Modules

The `modules` layer contains application-specific control logic.

Examples include:

-   hydraulic oil-temperature warning
-   wheel-loader cooling-fan control
-   vehicle-speed calculation
-   gear-display facade
-   speed monitoring
-   engine_overheat_protection

A typical module:

1.  reads one or more input signals
2.  processes control logic in `Update()`
3.  writes one or more output signals

The Arduino `OilTempWarning` module reads a temperature signal and produces a warning signal.

#### Engine Overheat Protection

The `EngineOverheatProtection` module monitors the engine-running state
and hydraulic oil temperature.

It activates overheat protection when the oil temperature remains above
the configured high threshold for the configured duration.

##### State Machine

```mermaid
stateDiagram-v2
    [*] --> IDLE

    IDLE --> COUNTING: Engine running and oil temperature >= high threshold
    COUNTING --> PROTECTED: Timer elapsed
    COUNTING --> IDLE: Oil temperature <= low threshold
    PROTECTED --> IDLE: Oil temperature <= low threshold

------------------------------------------------------------------------

### Utility

The `utility` layer contains reusable helper algorithms and timing components.

Current utilities include:

#### `Hysteresis`

Provides stable switching behavior using separate high and low thresholds.

Behavior:
``` text
Temperature >= high_threshold  -> warning ON
Temperature <= low_threshold  -> warning OFF
low_threshold < temperature < high_threshold -> keep previous state
```

This prevents the warning output from rapidly switching on and off when the input fluctuates near one threshold.

#### `IncrementTimer`

Provides non-blocking periodic timing using Arduino `millis()`.

It allows the application to execute control logic periodically without using `delay()`.

* Utilities are included only by the files that actually use them.

------------------------------------------------------------------------

### Assembly

The `assembly` layer creates modules and connects signals in the PC version of the project.

It acts as a system-wiring layer where concrete objects are instantiated and their dependencies are connected.

------------------------------------------------------------------------

### Tests

The `tests` folder contains PC-based manual programs separated into unit and module tests.
Inside of them, there are two test patterns: print based test and assert based test.

These tests are used to:

-   verify module logic
-   check signal behavior
-   test boundary conditions
-   validate modules before using them on embedded hardware

------------------------------------------------------------------------

### Arduino Application Layer

`OilTempWarningDemo.ino` is the Arduino application entry and integration layer.

It is responsible for:

-   creating the `Manager`
-   creating shared signals
-   creating and configuring modules
-   reading Arduino hardware inputs
-   running modules periodically
-   converting module outputs into hardware outputs
-   printing debug information through the serial port

The `.ino` file acts as the composition root of the Arduino application.

It knows the concrete classes because its responsibility is to assemble the complete system.

------------------------------------------------------------------------

## Arduino PROJECT Oil-Temperature Warning Demo

The Arduino demo has been successfully compiled, uploaded, and tested on an Arduino UNO-compatible board.

### Hardware

-   Arduino UNO
-   10 kΩ potentiometer
-   IN 4007 NPN
-   PN 2222
-   motor
-   1k resistence
-   built-in LED
-   jumper wires
-   breadboard

### Wiring

``` text
Potentiometer outer pin  -> Arduino 5V
Potentiometer middle pin -> Arduino A0
Potentiometer outer pin  -> Arduino GND
```

The two outer pins may be exchanged. This only reverses the direction in
which the ADC value increases.

The built-in Arduino LED is used as the warning output:

``` cpp
constexpr int WARNING_LED_PIN = LED_BUILTIN;
```

------------------------------------------------------------------------

## Arduino Data Flow

``` text
Potentiometer
    |
Arduino A0
    |
analogRead()
    |
ADC value: 0-1023
    |
Simulated oil temperature: 0-120 °C
    |
FloatSignal
    |
OilTempWarning::Update()
    |
Hysteresis
    |
BoolSignal
    |
Built-in LED
```

The ADC-to-temperature conversion is:

``` cpp
const float oil_temp_c =
    static_cast<float>(raw_adc)
    * MAX_SIMULATED_TEMP_C
    / 1023.0f;
```

The current simulation maps:

``` text
ADC 0    -> 0 °C
ADC 512  -> approximately 60 °C
ADC 1023 -> 120 °C
```

------------------------------------------------------------------------

## Hysteresis Behavior

The warning module uses separate activation and deactivation thresholds.

Example configuration:

``` cpp
constexpr float WARNING_LOW_THRESHOLD_C = 90.0f;
constexpr float WARNING_HIGH_THRESHOLD_C = 95.0f;
```

Behavior:

``` text
Initial state: OFF

Temperature rises:
89 °C -> OFF
93 °C -> OFF
95 °C -> ON
100 °C -> ON

Temperature falls:
94 °C -> remains ON
92 °C -> remains ON
90 °C -> OFF
```

This behavior prevents LED flickering near the warning threshold.

------------------------------------------------------------------------

## Periodic Execution

The Arduino application uses `IncrementTimer` instead of `delay()`.

Example:

``` cpp
if (!control_timer.IsExpired()) {
    return;
}
```

This means that the fast-running Arduino `loop()` skips the control task until the configured period has elapsed.

The control flow is:

``` text
Check timer
    |
Read analog input
    |
Update input signal
    |
Call Manager::UpdateAll()
    |
Read warning output
    |
Update LED
    |
Print debug information
```

------------------------------------------------------------------------

## Example Serial Output

``` text
OilTempWarningDemo started.
ADC: 765 | Oil temp: 89.74 C | Warning: OFF
ADC: 800 | Oil temp: 93.84 C | Warning: OFF
ADC: 833 | Oil temp: 97.71 C | Warning: ON
ADC: 884 | Oil temp: 103.70 C | Warning: ON
```

------------------------------------------------------------------------

## Build and Run PC Tests

Go to a test directory, create a build directory, configure CMake, and
build:

``` bash
cd Disc_name:\Embedded-Control-Architecture
cmake -S .\tests\module\"responding module floder" -B .\build\tests\module\"responding module folder"
cmake --build .\build\tests\module\"responding module folder" --config Debug (or Release)
cd .\build\tests\module\"responding module folder"\Debug (or Release)
cd .\"responding generated module .exe file"
```

Then run the generated executable.

On Windows, the executable may be generated inside a configuration directory such as:

``` text
Debug mode: 
    cmake --build .\build\tests\module\"test + module name.cpp" or "main.cpp" --config Debug
    cmake --build .\build\tests\unit\"test + module name.cpp" or "main.cpp" --config Debug
Release mode: 
    cmake --build .\build\tests\module\"test + module name.cpp" or "main.cpp" --config Release
    cmake --build .\build\tests\unit\"test + module name.cpp" or "main.cpp" --config Release
```
The main difference between "Debug" and "Release" is that "Debug" builds are intended for development and testing. They keep more debug information and use less optimization, and allow "assert()" to detect the details of failure tests. "Release" builds are intended for more optimized execution. They may disable "assert()" checks depends on compiler setting, so they are not used as the main mode for assertion-based tests.

Test command for add_test() items in CMakeList.txt:
    ctest --test-dir .\build\tests\unit -C Debug --output-on-failure
    explaination:
        - `ctest`: runs tests registered by CMake through `add_test()`.
        - `--test-dir`: specifies the build directory where the test configuration is located.
        - `-C Debug`: selects the `Debug` configuration for multi-configuration generators such as Visual Studio.
        - `--output-on-failure`: prints test output only when a test fails.

------------------------------------------------------------------------

## Arduino Usage

Open:

``` text
arduino_uno/OilTempWarningDemo/OilTempWarningDemo.ino
```

Then:

1.  Connect the Arduino UNO by USB.
2.  Select `Arduino UNO` in Arduino IDE.
3.  Select the correct serial port.
4.  Compile the sketch.
5.  Upload it to the board.
6.  Open the Serial Monitor.
7.  Select the baud rate configured in `Serial.begin()`.
8.  Rotate the potentiometer and observe the temperature, warning state, and built-in LED.

------------------------------------------------------------------------

## Design Notes

### Why `OilTempWarning` does not read A0 directly

The module reads a `FloatSignal` instead of directly calling:

``` cpp
analogRead(A0);
```

This separates hardware input from control logic.

The same module can later receive temperature data from a different source without changing its warning algorithm.

### Why `Hysteresis` is inside `OilTempWarning`

Hysteresis is part of the warning behavior:

``` text
turn ON at the high threshold
turn OFF at the low threshold
```

Therefore, it is reasonable for `OilTempWarning` to use `Hysteresis` internally.

### Why `IncrementTimer` is used by the `.ino` file

The current timer controls the execution period of the complete Arduino
application.

It is therefore part of application scheduling rather than the oil-temperature warning algorithm itself.

------------------------------------------------------------------------

## Current Status

Completed:

-   Basic module interface
-   Manager-based module registration
-   Shared typed signals
-   Signal validity handling
-   PC manual test targets
-   Arduino UNO application structure
-   Analog-input temperature simulation
-   Warning LED output
-   Non-blocking periodic execution
-   Hysteresis-based warning behavior
-   Successful hardware test on Arduino UNO
-   Relization of two modules in circuit (fan cooling control and oil temp waring)
-   Successful motor output
-   Add automated unit tests
-   Improve CMake organization

------------------------------------------------------------------------

## Future Improvements

Possible future improvements include:

-   Add input filtering for noisy analog signals
-   Add a buzzer warning output
-   Add an LCD temperature display
-   Add multiple modules with different update periods
-   Add CAN-style signal simulation
-   Add state-machine-based control logic
-   Separate platform-independent utilities from Arduino-specific utilities
-   Add architecture and signal-flow diagrams
-   Add real temperature-sensor support
-   Add fault handling for invalid or disconnected inputs

------------------------------------------------------------------------

## Notes

This is a learning-oriented embedded-control architecture project.

It is not intended to be a production-ready ECU framework.

The purpose is to practice:

-   C++ class design
-   object lifetime and references
-   header/source separation
-   module-based architecture
-   signal-based communication
-   manager-based execution
-   utility reuse
-   CMake build structure
-   Arduino hardware integration
-   embedded debugging

------------------------------------------------------------------------

## Troubleshooting

See [Troubleshooting Notes](docs/TROUBLESHOOTING.md). -\> Symptoms,
causes, diagnosis, and solutions

## Circuitnotes

See [Troubleshooting Notes](docs/CIRCUIT_NOTES.md). -\> Circuit design,
theory, formulas, measurements, and improvements

## License

This project is currently intended for personal learning and practice.
