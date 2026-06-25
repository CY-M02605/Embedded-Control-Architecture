# Troubleshooting Guide

This document records common hardware, software, build-system, and unit-testing issues encountered during the development of the `Embedded-Control-Architecture` project.

The current project includes:

* Arduino-based application code
* Modular C++ control logic
* Oil-temperature warning control
* Fan cooling control
* Hysteresis logic
* Non-blocking timer logic
* PC-side unit tests using CMake and CTest

---

## Table of Contents

* [1. Hardware and Electrical Issues](#1-hardware-and-electrical-issues)

  * [1.1 Serial Output Stops When the Motor Starts](#11-serial-output-stops-when-the-motor-starts)
  * [1.2 Arduino 5 V Rail Drops Under Motor Load](#12-arduino-5-v-rail-drops-under-motor-load)
  * [1.3 Correct PN2222 Motor Driver Wiring](#13-correct-pn2222-motor-driver-wiring)
  * [1.4 Flyback Diode Direction](#14-flyback-diode-direction)
  * [1.5 Why a 104 Capacitor Did Not Solve the Problem](#15-why-a-104-capacitor-did-not-solve-the-problem)
  * [1.6 Understanding VBE and VCE Measurements](#16-understanding-vbe-and-vce-measurements)
  * [1.7 Replacing the Motor with an LED for Testing](#17-replacing-the-motor-with-an-led-for-testing)

* [2. Arduino and Serial Communication](#2-arduino-and-serial-communication)

  * [2.1 Serial Monitor Cannot Connect to the COM Port](#21-serial-monitor-cannot-connect-to-the-com-port)
  * [2.2 Serial Monitor Baud Rate Mismatch](#22-serial-monitor-baud-rate-mismatch)
  * [2.3 Serial Output Freezes but the Program Was Uploaded Successfully](#23-serial-output-freezes-but-the-program-was-uploaded-successfully)

* [3. C++ and Application Logic](#3-c-and-application-logic)

  * [3.1 Too Many Initializers for Config](#31-too-many-initializers-for-config)
  * [3.2 Hysteresis Threshold Order Is Reversed](#32-hysteresis-threshold-order-is-reversed)
  * [3.3 Timer Output Is Always False](#33-timer-output-is-always-false)
  * [3.4 Timer Current Time Does Not Change](#34-timer-current-time-does-not-change)
  * [3.5 Assignment Used Instead of Comparison](#35-assignment-used-instead-of-comparison)
  * [3.6 Assert Cannot Compare a Void Return Value](#36-assert-cannot-compare-a-void-return-value)
  * [3.7 Class Scope Qualifier Used Inside the Class Body](#37-class-scope-qualifier-used-inside-the-class-body)
  * [3.8 Unused Config Structure](#38-unused-config-structure)
  * [3.9 Duplicating Production Code for Testing](#39-duplicating-production-code-for-testing)

* [4. CMake and Build Configuration](#4-cmake-and-build-configuration)

  * [4.1 CMake Cannot Find CMakeLists.txt](#41-cmake-cannot-find-cmakeliststxt)
  * [4.2 Relative Paths Depend on the Terminal Location](#42-relative-paths-depend-on-the-terminal-location)
  * [4.3 Header File Cannot Be Found](#43-header-file-cannot-be-found)
  * [4.4 Difference Between add_executable and target_include_directories](#44-difference-between-add_executable-and-target_include_directories)
  * [4.5 Include Directories Are Target-Specific](#45-include-directories-are-target-specific)
  * [4.6 IntelliSense Paths Are Not Compiler Paths](#46-intellisense-paths-are-not-compiler-paths)
  * [4.7 Source and Build Directories Were Mixed Up](#47-source-and-build-directories-were-mixed-up)
  * [4.8 CMake Command Contains an Extra Space in a Path](#48-cmake-command-contains-an-extra-space-in-a-path)

* [5. Unit Testing and CTest](#5-unit-testing-and-ctest)

  * [5.1 CTest Cannot Find the Test Executable](#51-ctest-cannot-find-the-test-executable)
  * [5.2 Assertion Failure Opens a Runtime Debug Window](#52-assertion-failure-opens-a-runtime-debug-window)
  * [5.3 Source Changes Require Rebuilding](#53-source-changes-require-rebuilding)
  * [5.4 Build and Test Commands Were Combined Incorrectly](#54-build-and-test-commands-were-combined-incorrectly)
  * [5.5 Standard Unit-Test Workflow](#55-standard-unit-test-workflow)
  * [5.6 Difference Between CMake and CTest](#56-difference-between-cmake-and-ctest)
  * [5.7 How to Confirm That a Test Really Works](#57-how-to-confirm-that-a-test-really-works)

* [6. Platform Dependencies and Testability](#6-platform-dependencies-and-testability)

  * [6.1 PC Tests Cannot Directly Use Arduino millis](#61-pc-tests-cannot-directly-use-arduino-millis)
  * [6.2 Passing the Current Time into the Timer](#62-passing-the-current-time-into-the-timer)
  * [6.3 Why a Separate Test-Only Timer Is Not Ideal](#63-why-a-separate-test-only-timer-is-not-ideal)
  * [6.4 Using unsigned long Versus std::uint32_t](#64-using-unsigned-long-versus-stduint32_t)

* [7. Recommended Project Structure](#7-recommended-project-structure)

* [8. Quick Diagnostic Checklist](#8-quick-diagnostic-checklist)

* [9. Lessons Learned](#9-lessons-learned)

---

# 1. Hardware and Electrical Issues

## 1.1 Serial Output Stops When the Motor Starts

### Symptom

The Arduino application works normally while the motor is disconnected.

When the motor is connected and switched on:

* Serial output stops
* The Arduino may restart
* The COM port may temporarily disconnect
* The Serial Monitor may stop updating
* The 5 V rail decreases

### Likely causes

* Motor startup current is too high
* The Arduino 5 V rail is overloaded
* The USB cable or USB port has excessive voltage drop
* The motor creates electrical noise
* The transistor or diode wiring is incorrect
* The transistor pin order is incorrect

### Recommended troubleshooting sequence

1. Disconnect the motor.
2. Confirm that the Serial Monitor works normally.
3. Replace the motor with an LED and resistor.
4. Confirm that the control output switches correctly.
5. Check the transistor pinout.
6. Check the flyback diode direction.
7. Measure the 5 V rail before and after motor startup.
8. Measure D8, VBE, and VCE.
9. Check whether any component becomes hot.
10. Consider using a separate motor power supply.

---

## 1.2 Arduino 5 V Rail Drops Under Motor Load

### Example measurements

| Condition                  |          Arduino 5 V |            D8 output |                  VBE |                  VCE |
| -------------------------- | -------------------: | -------------------: | -------------------: | -------------------: |
| Fan OFF                    | Approximately 4.97 V |    Approximately 0 V |    Approximately 0 V |    Approximately 5 V |
| Fan ON, motor disconnected | Approximately 4.96 V |  Approximately 4.9 V | Approximately 0.76 V |             Near 0 V |
| Fan ON, motor connected    |  Approximately 4.6 V | Approximately 4.38 V | Approximately 0.77 V | Approximately 0.15 V |

### Interpretation

The transistor appears to be operating correctly because:

```text
VBE ? 0.7?0.8 V
```

and:

```text
VCE(sat) ? 0.1?0.3 V
```

However, the Arduino 5 V rail drops significantly when the motor runs.

This suggests:

```text
Motor startup current
    v
Supply-voltage drop
    v
Arduino reset or serial instability
```

### Important note

A standard multimeter may not capture the lowest voltage during motor startup because the transient can be very short.

A portable oscilloscope with single-shot triggering is better for observing:

* D8 rising edge
* 5 V rail drop
* Motor startup transient
* Electrical noise

---

## 1.3 Correct PN2222 Motor Driver Wiring

The motor should not be driven directly from an Arduino I/O pin.

A recommended low-side switching circuit is:

```text
Arduino D8
    |
   1 kOhm
    |
PN2222 Base

PN2222 Emitter
    |
   GND

5 V
 |
Motor
 |
PN2222 Collector
```

### Current paths

When D8 is HIGH:

```text
Control current:
D8 -> 1 kOhm -> Base -> Emitter -> GND
```

```text
Motor current:
5 V -> Motor -> Collector -> Emitter -> GND
```

When D8 is LOW:

```text
Base current ? 0
Transistor OFF
Motor current ? 0
```

### Important checks

* Confirm the actual Base, Collector, and Emitter pinout.
* Do not assume that every PN2222 package uses the same pin order.
* Do not connect the motor directly to D8.
* Use a base resistor.
* Use a flyback diode.
* Use a common ground.

---

## 1.4 Flyback Diode Direction

The motor is an inductive load.

When the transistor switches off, the motor winding attempts to maintain current and can generate a high-voltage transient.

The flyback diode must be connected in reverse bias during normal operation.

Correct direction:

```text
Diode cathode, marked with a stripe
    -> Motor positive terminal / 5 V

Diode anode
    -> Motor negative terminal / transistor collector
```

During normal operation, the diode does not conduct.

When the transistor switches off, the diode provides a safe path for the remaining motor current.

### Incorrect diode direction

If the diode is reversed, it may conduct directly during normal operation and create an excessive current path.

Possible results:

* Arduino resets
* USB port protection activates
* Components become hot
* The diode or transistor may be damaged

---

## 1.5 Why a 104 Capacitor Did Not Solve the Problem

A `104` ceramic capacitor is approximately:

```text
0.1 ?F
```

It is useful for suppressing high-frequency brush noise from the motor.

It does not provide enough stored energy to support a large motor startup current.

Therefore:

```text
104 capacitor
-> Good for high-frequency noise suppression

Large electrolytic capacitor
-> Better for short supply-voltage dips
```

A larger capacitor such as:

```text
220 ?F
470 ?F
1000 ?F
```

may help reduce short voltage drops.

However, if the power source cannot provide enough current, a capacitor may not fully solve the problem.

---

## 1.6 Understanding VBE and VCE Measurements

### VBE

```text
VBE = VB - VE
```

For a silicon NPN transistor in conduction:

```text
VBE ? 0.6?0.8 V
```

A measured value near `0.77 V` is normal.

### VCE when the transistor is OFF

The transistor behaves like an open switch.

Motor current is approximately zero:

```text
I ? 0
```

Therefore, the motor voltage drop is approximately zero:

```text
VMotor = I * R ? 0
```

The collector is pulled toward the supply voltage through the motor:

```text
VC ? 5 V
VE ? 0 V
VCE ? 5 V
```

### VCE when the transistor is ON

When the transistor is saturated:

```text
VCE(sat) ? 0.1?0.3 V
```

A measured value around `0.15 V` indicates good saturation.

The motor voltage is approximately:

```text
VMotor = VSupply - VCE
```

Example:

```text
VMotor ? 4.60 V - 0.15 V
       ? 4.45 V
```

---

## 1.7 Replacing the Motor with an LED for Testing

The motor can be temporarily replaced by:

```text
LED + current-limiting resistor
```

Do not replace the motor with a bare LED.

Recommended connection:

```text
5 V
 |
220 Ohm to 1 kOhm resistor
 |
LED
 |
PN2222 Collector
PN2222 Emitter
 |
GND
```

This verifies:

* D8 control logic
* Base resistor
* PN2222 switching
* Collector-emitter current path

The flyback diode is not required while testing with an LED because the LED is not an inductive load.

---

# 2. Arduino and Serial Communication

## 2.1 Serial Monitor Cannot Connect to the COM Port

### Symptom

```text
Port monitor error:
Invalid serial port.
Could not connect to COM4 serial port.
```

### Possible causes

* Arduino was disconnected
* Windows assigned a different COM port
* Another application is using the port
* The wrong port is selected
* The USB cable does not support data transfer

### Solution

1. Disconnect and reconnect the Arduino.

2. Open:

   ```text
   Tools > Port
   ```

3. Select the currently available COM port.

4. Close and reopen the Serial Monitor.

5. Close any other serial tools.

6. Restart the Arduino IDE if necessary.

7. Confirm that the USB cable supports data.

---

## 2.2 Serial Monitor Baud Rate Mismatch

The program may contain:

```cpp
Serial.begin(9600);
```

The Serial Monitor must also be configured to:

```text
9600 baud
```

If the values do not match, output may appear unreadable or incorrect.

---

## 2.3 Serial Output Freezes but the Program Was Uploaded Successfully

A Serial Monitor failure does not always mean that the Arduino sketch failed to upload.

Possible situations:

* The program was uploaded successfully
* The Serial Monitor cannot reconnect
* The Arduino reset because of voltage drop
* The USB serial chip temporarily lost communication

Check:

* Whether the power LED remains stable
* Whether the board restarts
* Whether startup text is printed again
* Whether the COM port disappears and returns

---

# 3. C++ and Application Logic

## 3.1 Too Many Initializers for Config

### Symptom

```text
too many initializers for 'modules::OilTempWarning::Config'
```

### Cause

The structure contains one member:

```cpp
struct Config {
    utility::Hysteresis::Config hysteresis_config;
};
```

but two top-level values are provided:

```cpp
const modules::OilTempWarning::Config config {
    WARNING_LOW_THRESHOLD,
    WARNING_HIGH_THRESHOLD
};
```

### Solution A

Use nested braces:

```cpp
const modules::OilTempWarning::Config config {
    {
        WARNING_LOW_THRESHOLD,
        WARNING_HIGH_THRESHOLD
    }
};
```

### Solution B

Prefer business-level configuration:

```cpp
struct Config {
    float low_threshold;
    float high_threshold;
};
```

Then initialize normally:

```cpp
const modules::OilTempWarning::Config config {
    WARNING_LOW_THRESHOLD,
    WARNING_HIGH_THRESHOLD
};
```

---

## 3.2 Hysteresis Threshold Order Is Reversed

If the configuration is:

```cpp
struct Config {
    float low_threshold;
    float high_threshold;
};
```

this is incorrect:

```cpp
utility::Hysteresis hysteresis({
    85.0f,
    80.0f
});
```

It produces:

```text
low threshold  = 85
high threshold = 80
```

Correct version:

```cpp
utility::Hysteresis hysteresis({
    80.0f,
    85.0f
});
```

Expected behavior:

```text
Below 85 while OFF  -> remain OFF
At or above 85      -> switch ON
Between 80 and 85   -> preserve state
At or below 80      -> switch OFF
```

---

## 3.3 Timer Output Is Always False

Incorrect implementation:

```cpp
void Update(unsigned long current_ms) {
    if (current_ms - previous_ms_ >= interval_ms_) {
        output_ = true;
    }

    output_ = false;
}
```

Even when the condition is true, the final statement sets the output back to false.

Correct version:

```cpp
void Update(unsigned long current_ms) {
    if (current_ms - previous_ms_ >= interval_ms_) {
        previous_ms_ = current_ms;
        output_ = true;
    } else {
        output_ = false;
    }
}
```

Or return the result directly:

```cpp
bool Update(unsigned long current_ms) {
    if (current_ms - previous_ms_ < interval_ms_) {
        return false;
    }

    previous_ms_ = current_ms;
    return true;
}
```

---

## 3.4 Timer Current Time Does Not Change

A design that stores `current_ms` only in the constructor is not useful for repeated updates.

Example:

```cpp
IncrementTimerForAP(
    unsigned long interval_ms,
    unsigned long previous_ms,
    unsigned long current_ms
);
```

Repeated calls to `Update()` use the same current time.

Better design:

```cpp
void Update(unsigned long current_ms);
```

Example:

```cpp
timer.Update(0UL);
timer.Update(99UL);
timer.Update(100UL);
timer.Update(200UL);
```

---

## 3.5 Assignment Used Instead of Comparison

Incorrect:

```cpp
assert(increment_timer.OutputRef() = false);
```

Single equals means assignment.

Correct:

```cpp
assert(increment_timer.OutputRef() == false);
```

More concise:

```cpp
assert(!increment_timer.OutputRef());
```

For true:

```cpp
assert(increment_timer.OutputRef());
```

Rule:

```text
=   assignment
==  comparison
```

---

## 3.6 Assert Cannot Compare a Void Return Value

If:

```cpp
void Update(float value);
```

then this is invalid:

```cpp
assert(hysteresis.Update(70.0f) == false);
```

because `Update()` returns `void`.

Use:

```cpp
hysteresis.Update(70.0f);
assert(hysteresis.GetStatus() == false);
```

Or redesign `Update()` to return `bool`.

---

## 3.7 Class Scope Qualifier Used Inside the Class Body

Incorrect inside a class definition:

```cpp
void IncrementTimerForAP::Update() {
}
```

Correct:

```cpp
void Update() {
}
```

The class name qualifier is used only when defining the function outside the class:

```cpp
void IncrementTimerForAP::Update() {
}
```

---

## 3.8 Unused Config Structure

A structure such as:

```cpp
struct Config {
    unsigned long threshold_time;
};
```

should not remain if the constructor does not use it.

Either remove it or use it consistently:

```cpp
explicit IncrementTimer(Config config)
    : interval_ms_(config.threshold_time) {
}
```

Unused configuration increases confusion and maintenance cost.

---

## 3.9 Duplicating Production Code for Testing

Creating a second timer implementation only for PC tests may work, but it has a serious disadvantage:

```text
Production timer
!=
Test-only timer
```

The test may pass even if the real Arduino timer contains a bug.

A better design is to make the production timer testable by passing time as an argument.

---

# 4. CMake and Build Configuration

## 4.1 CMake Cannot Find CMakeLists.txt

### Symptom

```text
The source directory does not appear to contain CMakeLists.txt.
```

### Cause

The directory after `-S` does not contain `CMakeLists.txt`.

If the file is located at:

```text
tests/unit/CMakeLists.txt
```

the correct command from the repository root is:

```powershell
cmake -S .\tests\unit -B .\build\tests\unit
```

---

## 4.2 Relative Paths Depend on the Terminal Location

If the current terminal directory is:

```text
C:\Embedded-Control-Architecture\build
```

then:

```powershell
cmake -S tests/unit -B build/tests/unit
```

searches inside:

```text
C:\Embedded-Control-Architecture\build\tests\unit
```

Recommended approach:

```powershell
cd ..
```

Return to the repository root and run:

```powershell
cmake -S .\tests\unit -B .\build\tests\unit
```

---

## 4.3 Header File Cannot Be Found

### Symptom

```text
fatal error C1083:
Cannot open include file:
'increment_timer_for_arduino_project.h'
```

### Important rule

The compiler combines:

```text
Include search directory
+
Path written in #include
```

Example A:

```cpp
#include "increment_timer_for_arduino_project.h"
```

CMake:

```cmake
target_include_directories(test_increment_timer PRIVATE
    ${PROJECT_ROOT_DIR}/utility
)
```

Example B:

```cpp
#include "utility/increment_timer_for_arduino_project.h"
```

CMake:

```cmake
target_include_directories(test_increment_timer PRIVATE
    ${PROJECT_ROOT_DIR}
)
```

Both approaches are valid.

They must not be mixed incorrectly.

---

## 4.4 Difference Between add_executable and target_include_directories

```cmake
add_executable(test_hysteresis
    test_hysteresis.cpp
)
```

This means:

> Compile `test_hysteresis.cpp` and create an executable target named `test_hysteresis`.

```cmake
target_include_directories(test_hysteresis PRIVATE
    ${PROJECT_SOURCE_DIR}
)
```

This means:

> While compiling `test_hysteresis`, search for header files inside `${PROJECT_SOURCE_DIR}`.

Summary:

```text
add_executable()
-> Which source files should be compiled?

target_include_directories()
-> Where should the compiler search for header files?
```

---

## 4.5 Include Directories Are Target-Specific

This:

```cmake
target_include_directories(test_hysteresis PRIVATE
    ${PROJECT_SOURCE_DIR}
)
```

only applies to:

```text
test_hysteresis
```

It does not apply to:

```text
test_increment_timer
```

Each target must be configured separately.

---

## 4.6 IntelliSense Paths Are Not Compiler Paths

The file:

```text
.vscode/c_cpp_properties.json
```

configures VS Code IntelliSense.

It does not automatically configure CMake or MSVC.

A file may appear correctly in VS Code but still fail during compilation.

The actual compiler include paths must be configured in:

```cmake
target_include_directories(...)
```

---

## 4.7 Source and Build Directories Were Mixed Up

The source directory contains:

```text
CMakeLists.txt
.cpp
.h
```

The build directory contains generated files:

```text
CMakeCache.txt
CMakeFiles
.vcxproj
.exe
.pdb
```

Recommended layout:

```text
tests/unit
-> Source directory

build/tests/unit
-> Build directory
```

---

## 4.8 CMake Command Contains an Extra Space in a Path

Incorrect:

```powershell
cmake -S tests/ unit -B build/tests/unit
```

Because of the space, CMake treats `tests/` as the source directory.

Correct:

```powershell
cmake -S tests/unit -B build/tests/unit
```

Or:

```powershell
cmake -S .\tests\unit -B .\build\tests\unit
```

---

# 5. Unit Testing and CTest

## 5.1 CTest Cannot Find the Test Executable

### Symptom

```text
Could not find executable test_increment_time
```

### Cause

The executable target and the CTest command do not match.

Incorrect:

```cmake
add_executable(test_increment_timer
    test_increment_timer.cpp
)

add_test(
    NAME IncrementTimerTest
    COMMAND test_increment_time
)
```

Correct:

```cmake
add_test(
    NAME IncrementTimerTest
    COMMAND test_increment_timer
)
```

The command name must exactly match the executable target.

---

## 5.2 Assertion Failure Opens a Runtime Debug Window

In Debug mode, a failed `assert()` may display:

```text
Microsoft Visual C++ Runtime Library
Debug Error!
abort() has been called
```

The terminal usually reports:

```text
Assertion failed:
hysteresis.GetStatus() == true

file: test_hysteresis.cpp
line: 50
```

This means the test successfully detected a mismatch between expected and actual behavior.

Check:

* The reported line
* The input immediately before the assertion
* The current internal state
* Previous test steps
* Threshold order

---

## 5.3 Source Changes Require Rebuilding

After changing:

```text
.cpp
.cc
.h
```

run:

```powershell
cmake --build .\build\tests\unit --config Debug
ctest --test-dir .\build\tests\unit -C Debug --output-on-failure
```

Saving source code does not automatically update the existing executable.

---

## 5.4 Build and Test Commands Were Combined Incorrectly

Incorrect:

```powershell
cmake --build .\build\tests\unit -C Debug --output-on-failure
```

`--output-on-failure` belongs to CTest.

Correct build command:

```powershell
cmake --build .\build\tests\unit --config Debug
```

Correct test command:

```powershell
ctest --test-dir .\build\tests\unit -C Debug --output-on-failure
```

---

## 5.5 Standard Unit-Test Workflow

### Configure

```powershell
cmake -S .\tests\unit -B .\build\tests\unit
```

Meaning:

```text
-S
-> Source directory containing CMakeLists.txt

-B
-> Build output directory
```

### Build

```powershell
cmake --build .\build\tests\unit --config Debug
```

Meaning:

```text
Compile the test programs using the Debug configuration.
```

### Run

```powershell
ctest --test-dir .\build\tests\unit -C Debug --output-on-failure
```

Meaning:

```text
Run all registered Debug tests and print details if a test fails.
```

---

## 5.6 Difference Between CMake and CTest

CMake:

```text
Configures and builds the project.
```

CTest:

```text
Runs the registered tests.
```

Typical sequence:

```text
CMake configure
    v
CMake build
    v
CTest run
```

---

## 5.7 How to Confirm That a Test Really Works

A test should be intentionally broken once.

Example:

Correct:

```cpp
assert(hysteresis.GetStatus() == true);
```

Temporarily change to:

```cpp
assert(hysteresis.GetStatus() == false);
```

Rebuild and run.

The test should fail.

Then restore the correct expectation.

This confirms that the test is actually checking behavior.

---

# 6. Platform Dependencies and Testability

## 6.1 PC Tests Cannot Directly Use Arduino millis

Arduino code may depend on:

```cpp
#include <Arduino.h>
millis()
```

A normal PC/MSVC environment does not provide these APIs.

Typical errors:

```text
Arduino.h: No such file or directory
```

or:

```text
millis: identifier not found
```

---

## 6.2 Passing the Current Time into the Timer

Preferred design:

```cpp
class IncrementTimer {
public:
    explicit IncrementTimer(
        unsigned long interval_ms,
        unsigned long initial_time_ms = 0
    )
        : interval_ms_(interval_ms),
          previous_ms_(initial_time_ms) {
    }

    bool Update(unsigned long current_ms) {
        if (current_ms - previous_ms_ < interval_ms_) {
            return false;
        }

        previous_ms_ = current_ms;
        return true;
    }

private:
    unsigned long interval_ms_;
    unsigned long previous_ms_;
};
```

Arduino usage:

```cpp
if (timer.Update(millis())) {
    manager.Update();
}
```

PC test:

```cpp
utility::IncrementTimer timer(100UL);

assert(timer.Update(0UL) == false);
assert(timer.Update(99UL) == false);
assert(timer.Update(100UL) == true);
assert(timer.Update(101UL) == false);
assert(timer.Update(200UL) == true);
```

---

## 6.3 Why a Separate Test-Only Timer Is Not Ideal

A separate class such as:

```text
IncrementTimerForAP
```

can be tested on PC, but it does not prove that the Arduino production timer works correctly.

Possible divergence:

```text
Production code changes
Test-only code does not change
```

or:

```text
Test-only code is fixed
Production code remains broken
```

The best unit test verifies the same implementation used in production.

---

## 6.4 Using unsigned long Versus std::uint32_t

Both can work.

For Arduino:

```cpp
unsigned long
```

is natural because `millis()` returns `unsigned long`.

Advantages:

* Matches Arduino APIs
* Easy to understand
* Usually 32-bit on Arduino Uno

`std::uint32_t` provides an explicitly fixed 32-bit width.

Use it when exact width across platforms is important.

For this project, `unsigned long` is acceptable.

---

# 7. Recommended Project Structure

```text
Embedded-Control-Architecture/
|???? Arduino_project/
|   `???? OilTempWarningDemo/
|       |???? OilTempWarningDemo.ino
|       `???? src/
|           |???? framework/
|           |???? modules/
|           |???? signals/
|           `???? utility/
|???? tests/
|   |???? module/
|   `???? unit/
|       |???? CMakeLists.txt
|       |???? test_hysteresis.cpp
|       `???? test_increment_timer.cpp
|???? utility/
|---- build/ (not pushed into repository)
|???? docs/
`???? README.md
```

### Why tests should remain outside the Arduino sketch directory

This prevents:

* Arduino from compiling PC-only test code
* Conflicts with Arduino's own `main()`
* PC-only headers being included in Arduino builds
* Hardware application code and test code becoming mixed

---

# 8. Quick Diagnostic Checklist

When a build, test, or hardware experiment fails, check the following:

1. Is the terminal located at the repository root?
2. Does the `-S` directory contain `CMakeLists.txt`?
3. Does the `-B` directory point to the intended build folder?
4. Was CMake reconfigured after editing `CMakeLists.txt`?
5. Was the project rebuilt after editing `.cpp`, `.cc`, or `.h` files?
6. Does every test target have its own include directories?
7. Does the `#include` path match the configured include path?
8. Does `add_test(COMMAND ...)` match the executable target exactly?
9. Is `ctest` used to run tests?
10. Does the failed assertion report a specific file and line?
11. Are hysteresis thresholds in the correct order?
12. Does the tested class depend directly on Arduino-only APIs?
13. Is the motor isolated from the Arduino I/O pin?
14. Is the flyback diode connected in the correct direction?
15. Are the transistor pins confirmed?
16. Does the 5 V rail remain stable during motor startup?
17. Does the system work normally when the motor is disconnected?
18. Can the motor be temporarily replaced with an LED and resistor?
19. Are Arduino and external power grounds connected together?
20. Is any component becoming hot?

---

# 9. Lessons Learned

* Hardware power problems can appear as software failures.
* Serial communication failures may be caused by supply instability.
* Measurements are more reliable than assumptions.
* A 104 capacitor suppresses noise but does not replace a power supply.
* Transistor pinouts must be verified.
* CMake source and build directories serve different purposes.
* Include paths must match the path written in `#include`.
* CMake include settings are target-specific.
* IntelliSense configuration is not compiler configuration.
* Source changes require rebuilding.
* CTest runs tests but does not compile them.
* Failed assertions are useful diagnostic results.
* Unit tests should test production implementations.
* Platform-specific functions should be separated from testable logic.
* Non-blocking timing logic is easier to test when time is passed as data.
* Troubleshooting should separate hardware, software, build-system, and test-system problems.
