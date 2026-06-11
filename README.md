# Embedded Control Architecture

A C++ practice project for learning modular embedded control software architecture.

This project focuses on how embedded control software can be organized into independent modules, shared signals, a simple manager-based scheduler, and separated test/demo targets. It includes both PC-based simulation tests and an Arduino UNO demo.

## Project Goals

The main goals of this project are:

* Practice C++ modular design for embedded control logic
* Understand the relationship between modules, signals, manager, utility, and assembly layers
* Build small control modules with clear input/output signal interfaces
* Practice CMake-based build and manual testing on PC
* Port part of the control logic to Arduino UNO as a simple hardware demo

## Project Structure

```text
Embedded-Control-Architecture/
├── assembly/
│   ├── instantiation.h
│   └── instantiation_practice.h
│
├── framework/
│   ├── manager.h
│   └── module_interface.h
│
├── modules/
│   ├── gear_display_facade/
│   ├── hydraulic_oil_warning/
│   ├── instantiation_practice/
│   ├── wl_cooling_fan_control/
│   └── wl_vehicle_speed/
│
├── signals/
│   └── signal.h
│
├── utility/
│   ├── hysteresis.h
│   └── increment_timer.h
│
├── tests/
│   ├── gear_display_facade/
│   ├── hydraulic_oil_warning/
│   ├── instantiation_practice/
│   ├── wl_cooling_fan_control/
│   └── wl_vehicle_speed/
│
└── arduino_uno/
    └── OilTempWarningDemo/
        ├── OilTempWarningDemo.ino
        ├── framework/
        ├── modules/
        ├── signals/
        └── utility/
```

## Architecture Overview

The project is organized around several basic layers.

### framework

The `framework` layer provides the basic execution structure.

* `ModuleInterface` defines a common interface for control modules.
* `Manager` stores registered modules and calls their `Update()` functions.

This allows different modules to be updated through the same scheduler-like mechanism.

### signals

The `signals` layer provides simple signal objects used to pass values between modules.

A signal usually contains:

* a value, such as `bool`, `int`, or `float`
* a validity status

Examples:

```cpp
signals::FloatSignal oil_temp;
signals::BoolSignal warning_output;
```

### modules

The `modules` layer contains control logic modules.

Examples include:

* hydraulic oil temperature warning
* wheel loader cooling fan control
* vehicle speed calculation
* gear display facade
* speed monitor

Each module is designed to read input signals, process logic in `Update()`, and provide output signals.

### utility

The `utility` layer contains reusable helper components.

Examples:

* `Hysteresis`
* `IncrementTimer`

These utilities are used by modules to implement common embedded control logic.

### assembly

The `assembly` layer is used to instantiate modules and connect signals.

It represents the system wiring layer, where modules are created and connected together.

### tests

The `tests` folder contains manual test programs for different modules.

Each test is used to verify module behavior on a PC before moving logic to embedded hardware.

### arduino_uno

The `arduino_uno` folder contains an Arduino UNO demo version.

The current demo is:

```text
arduino_uno/OilTempWarningDemo/
```

This demo is intended to run on Arduino UNO and demonstrates how part of the control logic can be ported from the PC simulation project to a simple embedded target.

## Arduino UNO Demo

The Arduino UNO demo uses the same general architecture:

```text
input signal -> control module -> output signal -> hardware output
```

Example concept:

* Use analog input to simulate oil temperature
* Convert ADC value into a temperature value
* Feed the value into an oil temperature warning module
* Turn on an LED when the warning condition is active

The Arduino version uses `setup()` and `loop()` instead of a normal PC `main()` function.

## Build and Run PC Tests

Go to a test folder, create a build directory, configure CMake, and build:

```bash
cd tests/instantiation_practice
mkdir build
cd build
cmake ..
cmake --build .
```

Then run the generated executable.

On Windows, the executable may be generated under a build configuration folder depending on the selected generator.

## Arduino Usage

Open the following file with Arduino IDE:

```text
arduino_uno/OilTempWarningDemo/OilTempWarningDemo.ino
```

Then:

1. Select board: Arduino UNO
2. Select the correct serial port
3. Compile the sketch
4. Upload it to the board

## Notes

This project is a learning-oriented embedded control architecture practice project.

It is not intended to be a production-ready ECU software framework. The purpose is to practice:

* C++ class design
* header/source separation
* module-based architecture
* signal-based communication
* manager-based update scheduling
* CMake build structure
* Arduino UNO porting

## Future Improvements

Possible future improvements:

* Add more unit/manual tests
* Improve CMake organization
* Add more Arduino demos
* Add CAN-style signal simulation
* Add more realistic vehicle control examples
* Separate PC simulation and embedded target code more clearly
* Add diagrams for module connections

## License

This project is currently for personal learning and practice.
