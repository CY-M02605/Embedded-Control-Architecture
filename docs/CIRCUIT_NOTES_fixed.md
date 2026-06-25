# Circuit Design Notes

This document describes the circuit architecture, component functions, electrical principles, calculations, measurements, and improvement plans for the `Embedded-Control-Architecture` project.

The current prototype simulates a simple vehicle oil-temperature monitoring and cooling system using an Arduino Uno.

---

## Table of Contents

* [1. System Overview](#1-system-overview)

  * [1.1 System Purpose](#11-system-purpose)
  * [1.2 Signal Flow](#12-signal-flow)
  * [1.3 Current Control Thresholds](#13-current-control-thresholds)

* [2. Hardware Components](#2-hardware-components)

  * [2.1 Main Components](#21-main-components)
  * [2.2 Resistor and Capacitor Values](#22-resistor-and-capacitor-values)

* [3. Input Circuit](#3-input-circuit)

  * [3.1 Potentiometer as a Simulated Temperature Sensor](#31-potentiometer-as-a-simulated-temperature-sensor)
  * [3.2 Voltage Divider Principle](#32-voltage-divider-principle)
  * [3.3 ADC Conversion](#33-adc-conversion)

* [4. Warning Output Circuit](#4-warning-output-circuit)

  * [4.1 LED Connection](#41-led-connection)
  * [4.2 LED Current Calculation](#42-led-current-calculation)

* [5. Fan Motor Driver Circuit](#5-fan-motor-driver-circuit)

  * [5.1 Why the Motor Cannot Be Driven Directly](#51-why-the-motor-cannot-be-driven-directly)
  * [5.2 Low-Side Switching Architecture](#52-low-side-switching-architecture)
  * [5.3 PN2222 Terminal Functions](#53-pn2222-terminal-functions)
  * [5.4 Base Resistor](#54-base-resistor)
  * [5.5 Flyback Diode](#55-flyback-diode)
  * [5.6 Motor Noise-Suppression Capacitor](#56-motor-noise-suppression-capacitor)
  * [5.7 Bulk Electrolytic Capacitor](#57-bulk-electrolytic-capacitor)

* [6. Circuit Operating States](#6-circuit-operating-states)

  * [6.1 Transistor OFF State](#61-transistor-off-state)
  * [6.2 Transistor ON State](#62-transistor-on-state)
  * [6.3 Transistor Switching OFF](#63-transistor-switching-off)

* [7. Voltage and Current Calculations](#7-voltage-and-current-calculations)

  * [7.1 Base Current](#71-base-current)
  * [7.2 Collector Current](#72-collector-current)
  * [7.3 Motor Voltage](#73-motor-voltage)
  * [7.4 Transistor Power Dissipation](#74-transistor-power-dissipation)
  * [7.5 Motor Startup Current](#75-motor-startup-current)

* [8. Understanding Important Transistor Voltages](#8-understanding-important-transistor-voltages)

  * [8.1 Base-Emitter Voltage](#81-base-emitter-voltage)
  * [8.2 Collector-Emitter Voltage in the OFF State](#82-collector-emitter-voltage-in-the-off-state)
  * [8.3 Collector-Emitter Voltage in the ON State](#83-collector-emitter-voltage-in-the-on-state)
  * [8.4 Why D8 Voltage Drops Under Load](#84-why-d8-voltage-drops-under-load)

* [9. Power Supply and Grounding](#9-power-supply-and-grounding)

  * [9.1 Arduino 5 V Rail](#91-arduino-5-v-rail)
  * [9.2 Common Ground](#92-common-ground)
  * [9.3 Separate Motor Power Supply](#93-separate-motor-power-supply)
  * [9.4 Why Two 5 V Outputs Should Not Be Directly Paralleled](#94-why-two-5-v-outputs-should-not-be-directly-paralleled)

* [10. Noise and Protection](#10-noise-and-protection)

  * [10.1 Motor Brush Noise](#101-motor-brush-noise)
  * [10.2 Inductive Voltage Spikes](#102-inductive-voltage-spikes)
  * [10.3 Power-Rail Voltage Drop](#103-power-rail-voltage-drop)
  * [10.4 Function of Different Capacitors](#104-function-of-different-capacitors)

* [11. Measurement Plan](#11-measurement-plan)

  * [11.1 5 V Rail Measurement](#111-5-v-rail-measurement)
  * [11.2 D8 Measurement](#112-d8-measurement)
  * [11.3 VBE Measurement](#113-vbe-measurement)
  * [11.4 VCE Measurement](#114-vce-measurement)
  * [11.5 Motor Voltage Measurement](#115-motor-voltage-measurement)
  * [11.6 Motor Current Measurement](#116-motor-current-measurement)
  * [11.7 Oscilloscope Measurement Plan](#117-oscilloscope-measurement-plan)

* [12. Recorded Measurements](#12-recorded-measurements)

  * [12.1 Measurement Table](#121-measurement-table)
  * [12.2 Interpretation](#122-interpretation)

* [13. Hardware Verification Strategy](#13-hardware-verification-strategy)

  * [13.1 LED Substitution Test](#131-led-substitution-test)
  * [13.2 Separating Software and Hardware Problems](#132-separating-software-and-hardware-problems)
  * [13.3 Recommended Verification Order](#133-recommended-verification-order)

* [14. Recommended Circuit Improvements](#14-recommended-circuit-improvements)

  * [14.1 Separate Motor Supply](#141-separate-motor-supply)
  * [14.2 Logic-Level MOSFET](#142-logic-level-mosfet)
  * [14.3 Improved Decoupling](#143-improved-decoupling)
  * [14.4 Better Wiring Layout](#144-better-wiring-layout)
  * [14.5 Future Current Sensing](#145-future-current-sensing)

* [15. Safety Notes](#15-safety-notes)

  * [15.1 Low-Voltage Prototype Safety](#151-low-voltage-prototype-safety)
  * [15.2 Mains Voltage Warning](#152-mains-voltage-warning)
  * [15.3 Capacitor Polarity](#153-capacitor-polarity)
  * [15.4 Current Measurement Safety](#154-current-measurement-safety)

* [16. Lessons Learned](#16-lessons-learned)

---

# 1. System Overview

## 1.1 System Purpose

The prototype simulates an oil-temperature monitoring and cooling system.

The system performs the following functions:

* Reads a simulated oil-temperature input
* Activates a cooling fan at a high temperature
* Deactivates the cooling fan after the temperature falls sufficiently
* Activates a warning output at a higher temperature
* Uses hysteresis to prevent rapid ON/OFF switching

---

## 1.2 Signal Flow

```text
Potentiometer
    |
    v
Arduino analog input
    |
    v
Oil temperature signal
    |
    +----------------------+
    |                      |
    v                      v
FanCoolingControl     OilTempWarning
    |                      |
    v                      v
Motor driver          Warning LED
```

The potentiometer represents a simulated sensor.

The Arduino converts the analog voltage into a temperature value.

The same oil-temperature signal is used by both control modules.

---

## 1.3 Current Control Thresholds

Example thresholds:

```text
Fan ON threshold:      85 deg C
Fan OFF threshold:     80 deg C

Warning ON threshold:  95 deg C
Warning OFF threshold: 90 deg C
```

The difference between the ON and OFF thresholds creates hysteresis.

This prevents the output from rapidly changing state when the temperature fluctuates near one threshold.

---

# 2. Hardware Components

## 2.1 Main Components

| Component              | Function                                     |
| ---------------------- | -------------------------------------------- |
| Arduino Uno            | Main controller                              |
| Potentiometer          | Simulated temperature sensor                 |
| LED                    | Warning indicator                            |
| DC motor               | Cooling fan load                             |
| PN2222 NPN transistor  | Low-side motor switch                        |
| 1N4007 diode           | Flyback protection                           |
| 220 Ohm resistor         | LED current limiting                         |
| 1 kOhm resistor          | Transistor base-current limiting             |
| 104 ceramic capacitor  | Motor-noise suppression                      |
| Electrolytic capacitor | Supply-voltage stabilization                 |
| Multimeter             | Voltage, current, and continuity measurement |

---

## 2.2 Resistor and Capacitor Values

Typical values used in the prototype:

```text
LED resistor:          220 Ohm
Base resistor:         1 kOhm
Ceramic capacitor:     104 = 0.1 uF
Bulk capacitor:        220-1000 uF
```

The electrolytic capacitor should have a voltage rating above the supply voltage.

For a 5 V circuit, a capacitor rated at 10 V or higher is appropriate.

---

# 3. Input Circuit

## 3.1 Potentiometer as a Simulated Temperature Sensor

The potentiometer is used as a controllable voltage source.

Connection:

```text
5 V
 |
Potentiometer
 |
GND
```

The center terminal is connected to the Arduino analog input:

```text
Potentiometer terminal 1 -> 5 V
Potentiometer center     -> A0
Potentiometer terminal 3 -> GND
```

Rotating the potentiometer changes the voltage at A0.

---

## 3.2 Voltage Divider Principle

The potentiometer acts as a variable voltage divider.

The output voltage is approximately:

```text
Vout = Vin * Rbottom / (Rtop + Rbottom)
```

When the knob moves, the resistance ratio changes.

Therefore, the voltage at the center terminal changes between approximately:

```text
0 V to 5 V
```

---

## 3.3 ADC Conversion

The Arduino Uno ADC normally produces values from:

```text
0 to 1023
```

Example:

```cpp
const int adc_value = analogRead(A0);
```

If the simulated maximum temperature is `120 deg C`:

```cpp
const float oil_temp =
    static_cast<float>(adc_value) *
    120.0f / 1023.0f;
```

Example values:

```text
ADC = 0
Temperature approx 0 deg C

ADC = 512
Temperature approx 60 deg C

ADC = 1023
Temperature approx 120 deg C
```

The potentiometer does not measure real temperature.

It is only a controllable test input.

---

# 4. Warning Output Circuit

## 4.1 LED Connection

Recommended LED connection:

```text
Arduino output
    |
220 Ohm resistor
    |
LED anode
LED cathode
    |
GND
```

Typical LED polarity:

```text
Long lead  -> Anode
Short lead -> Cathode
Flat side  -> Cathode
```

---

## 4.2 LED Current Calculation

The LED current is approximately:

```text
ILED = (Voutput - VLED) / R
```

Example:

```text
Voutput = 5 V
VLED    = 2 V
R       = 220 Ohm
```

Therefore:

```text
ILED = (5 - 2) / 220
ILED approx 13.6 mA
```

The resistor is required to prevent excessive LED current.

---

# 5. Fan Motor Driver Circuit

## 5.1 Why the Motor Cannot Be Driven Directly

An Arduino output pin is designed mainly for control signals.

A DC motor may require:

* Higher operating current
* Much higher startup current
* Protection against inductive voltage spikes

Direct connection may cause:

* Output-pin overcurrent
* Arduino reset
* Serial disconnection
* Voltage drop
* Permanent I/O damage

The Arduino should control a switching device rather than supplying the motor current directly.

---

## 5.2 Low-Side Switching Architecture

The current prototype uses an NPN transistor as a low-side switch.

```text
                 +5 V
                  |
                Motor
                  |
             Collector
              PN2222
             Emitter
                  |
                 GND
```

Control connection:

```text
Arduino D8
    |
   1 kOhm
    |
PN2222 Base
```

The transistor is placed between the motor and ground.

This is called low-side switching.

---

## 5.3 PN2222 Terminal Functions

The PN2222 contains three terminals:

```text
B = Base
C = Collector
E = Emitter
```

Their functions are:

```text
Base
-> Receives the control current

Collector
-> Carries the motor current

Emitter
-> Returns current to ground
```

The physical pin order can vary by package and manufacturer.

Always verify the actual datasheet or component identification.

---

## 5.4 Base Resistor

The base resistor limits the current flowing from D8 into the transistor base.

Connection:

```text
D8 -> 1 kOhm -> Base
```

The base-emitter junction behaves similarly to a diode.

Typical conducting voltage:

```text
VBE approx 0.6-0.8 V
```

Without the resistor, excessive base current could damage:

* The Arduino output pin
* The transistor base-emitter junction

---

## 5.5 Flyback Diode

The motor is an inductive load.

When the transistor turns OFF, the winding attempts to maintain its current and can generate a high-voltage transient.

Correct diode connection:

```text
Stripe side / cathode
-> Motor positive terminal / 5 V

Non-stripe side / anode
-> Motor negative terminal / collector
```

Normal operation:

```text
Diode reverse-biased
-> No current through diode
```

Switch-off transient:

```text
Motor current continues
-> Diode becomes forward-biased
-> Current circulates safely
```

---

## 5.6 Motor Noise-Suppression Capacitor

A `104` ceramic capacitor is approximately:

```text
0.1 uF
```

It can be connected directly across the motor terminals:

```text
Motor positive
    |
   104
    |
Motor negative
```

Its main purpose is to suppress high-frequency noise generated by the motor brushes.

It does not provide enough stored energy to solve a large startup-current problem.

---

## 5.7 Bulk Electrolytic Capacitor

A bulk capacitor can be connected across the motor supply:

```text
Capacitor positive -> 5 V
Capacitor negative -> GND
```

Typical values:

```text
220 uF
470 uF
1000 uF
```

Its purpose is to reduce short voltage dips.

It may help during transient current demand, but it cannot compensate for a power source with insufficient current capability.

---

# 6. Circuit Operating States

## 6.1 Transistor OFF State

When:

```text
D8 = LOW
```

then:

```text
Base current approx 0
Transistor OFF
Collector current approx 0
Motor OFF
```

The transistor behaves approximately like an open switch.

---

## 6.2 Transistor ON State

When:

```text
D8 = HIGH
```

base current flows:

```text
D8
-> Base resistor
-> Base
-> Emitter
-> GND
```

Motor current flows:

```text
5 V
-> Motor
-> Collector
-> Emitter
-> GND
```

The transistor should enter saturation and behave approximately like a closed switch.

---

## 6.3 Transistor Switching OFF

When D8 changes from HIGH to LOW:

```text
Base current stops
Transistor switches OFF
```

The motor winding attempts to maintain current.

The flyback diode provides a safe current path until the stored magnetic energy decays.

---

# 7. Voltage and Current Calculations

## 7.1 Base Current

Base current:

```text
IB = (VD8 - VBE) / RB
```

Example measured values:

```text
VD8 = 4.38 V
VBE = 0.77 V
RB  = 1000 Ohm
```

Calculation:

```text
IB = (4.38 - 0.77) / 1000
IB = 3.61 / 1000
IB ? 3.61 mA
```

---

## 7.2 Collector Current

The collector current is approximately equal to the motor current:

```text
IC approx IMotor
```

For a motor that is not rotating:

```text
IMotor,start approx
(VSupply - VCE) / Rwinding
```

For a rotating motor:

```text
IMotor approx
(VSupply - VCE - Eback) / Rwinding
```

where `Eback` is the motor back electromotive force.

---

## 7.3 Motor Voltage

For the low-side switching circuit:

```text
VSupply = VMotor + VCE
```

Therefore:

```text
VMotor = VSupply - VCE
```

Example:

```text
VSupply = 4.60 V
VCE     = 0.15 V
```

Then:

```text
VMotor approx 4.45 V
```

---

## 7.4 Transistor Power Dissipation

The approximate transistor power is:

```text
PTransistor = VCE * IC
```

Example:

```text
VCE = 0.15 V
IC  = 0.10 A
```

Then:

```text
P ? 0.015 W
```

A lower VCE in the ON state reduces transistor heating.

---

## 7.5 Motor Startup Current

At startup:

```text
Motor speed approx 0
Back EMF approx 0
```

Therefore, the current is primarily limited by:

* Motor winding resistance
* Wire resistance
* Transistor voltage drop
* Power-source internal resistance

Startup current can be much larger than normal running current.

---

# 8. Understanding Important Transistor Voltages

## 8.1 Base-Emitter Voltage

Definition:

```text
VBE = VB - VE
```

Typical conducting value:

```text
0.6-0.8 V
```

Measured value:

```text
Approximately 0.77 V
```

This indicates that the base-emitter junction is forward-biased.

---

## 8.2 Collector-Emitter Voltage in the OFF State

Definition:

```text
VCE = VC - VE
```

When the transistor is OFF:

```text
Collector current approx 0
```

The motor has almost no voltage drop because little current flows.

Therefore:

```text
VC approx 5 V
VE approx 0 V
VCE approx 5 V
```

A high VCE does not imply high transistor power if current is nearly zero.

---

## 8.3 Collector-Emitter Voltage in the ON State

When the transistor is saturated:

```text
VCE(sat) ? 0.1-0.3 V
```

Measured value:

```text
Approximately 0.15 V
```

This indicates good switch conduction.

---

## 8.4 Why D8 Voltage Drops Under Load

The D8 HIGH voltage depends on the Arduino supply voltage.

When the motor causes the 5 V rail to drop:

```text
5 V rail decreases
    v
D8 HIGH voltage also decreases
```

Additional voltage drop can come from:

* Arduino output-stage resistance
* Breadboard contact resistance
* Wire resistance
* Ground bounce
* Electrical noise

---

# 9. Power Supply and Grounding

## 9.1 Arduino 5 V Rail

The Arduino 5 V rail is not an ideal power source.

It has limited current capability and internal resistance.

Large loads may cause:

* Voltage drop
* Reset
* Serial instability
* Incorrect logic levels

---

## 9.2 Common Ground

If an external power supply is used:

```text
Arduino GND
External power GND
Transistor emitter
```

must be connected together.

This ensures that D8 and VBE share the same voltage reference.

---

## 9.3 Separate Motor Power Supply

Recommended arrangement:

```text
Arduino USB supply
-> Arduino only

External 5 V supply
-> Motor only
```

Connection:

```text
External 5 V -> Motor positive
Motor negative -> Collector
Emitter -> External GND
Arduino GND -> External GND
D8 -> Base resistor -> Base
```

---

## 9.4 Why Two 5 V Outputs Should Not Be Directly Paralleled

Two separate 5 V power outputs may not have exactly the same voltage.

Directly connecting them may cause current to flow from one power source into the other.

Unless parallel operation is explicitly supported, do not connect:

```text
Arduino 5 V output
directly to
External 5 V output
```

Use a shared ground only.

---

# 10. Noise and Protection

## 10.1 Motor Brush Noise

Small brushed motors generate high-frequency electrical noise.

Possible effects:

* Serial instability
* ADC fluctuation
* Arduino reset
* Random signal errors

A ceramic capacitor across the motor helps suppress this noise.

---

## 10.2 Inductive Voltage Spikes

The motor winding stores magnetic energy.

When current is interrupted, this energy produces a voltage spike.

The flyback diode provides a safe discharge path.

---

## 10.3 Power-Rail Voltage Drop

Large motor current causes voltage drop across:

* USB cable resistance
* Breadboard contacts
* Arduino regulator or protection circuit
* Wires
* Power-source internal resistance

This can reduce both:

```text
Arduino 5 V rail
D8 HIGH output voltage
```

---

## 10.4 Function of Different Capacitors

### 104 ceramic capacitor

```text
Main function:
High-frequency noise suppression
```

### Large electrolytic capacitor

```text
Main function:
Short-term energy storage and voltage support
```

The two capacitor types serve different purposes and may be used together.

---

# 11. Measurement Plan

## 11.1 5 V Rail Measurement

Measure:

```text
Red probe  -> Arduino 5 V
Black probe -> Arduino GND
```

Record:

* Motor OFF
* Motor startup
* Motor running

---

## 11.2 D8 Measurement

Measure:

```text
Red probe  -> D8
Black probe -> GND
```

Expected:

```text
Fan OFF -> Approximately 0 V
Fan ON  -> Near supply voltage
```

---

## 11.3 VBE Measurement

Measure:

```text
Red probe  -> Base
Black probe -> Emitter
```

Expected during conduction:

```text
Approximately 0.6-0.8 V
```

---

## 11.4 VCE Measurement

Measure:

```text
Red probe  -> Collector
Black probe -> Emitter
```

Expected:

```text
Transistor OFF -> Near supply voltage
Transistor ON  -> Approximately 0.1-0.3 V
```

---

## 11.5 Motor Voltage Measurement

Measure directly across the motor terminals.

Expected during operation:

```text
VMotor approx VSupply - VCE
```

---

## 11.6 Motor Current Measurement

A current meter must be connected in series.

Correct:

```text
5 V
-> Ammeter
-> Motor
-> Transistor
-> GND
```

Incorrect:

```text
Ammeter directly between 5 V and GND
```

The incorrect connection creates a short circuit.

---

## 11.7 Oscilloscope Measurement Plan

Recommended dual-channel setup:

```text
CH1 -> Arduino 5 V rail
CH2 -> D8
Both ground clips -> Arduino GND
```

Suggested settings:

```text
Coupling: DC
Trigger source: D8
Trigger edge: Rising
Trigger mode: Single
Time base: Approximately 1-10 ms/div
```

This can reveal whether the 5 V rail drops immediately after the motor is switched on.

---

# 12. Recorded Measurements

## 12.1 Measurement Table

| Condition                  |            5 V Rail |                   D8 |    VBE |                  VCE |
| -------------------------- | ------------------: | -------------------: | -----: | -------------------: |
| Fan OFF                    |              4.97 V |                  0 V |    0 V |    Approximately 5 V |
| Fan ON, motor disconnected |              4.96 V |  Approximately 4.9 V | 0.76 V |             Near 0 V |
| Fan ON, motor connected    | Approximately 4.6 V | Approximately 4.38 V | 0.77 V | Approximately 0.15 V |

---

## 12.2 Interpretation

The measured VBE and VCE values indicate that the PN2222 is probably switching correctly.

The most significant abnormal result is:

```text
5 V rail:
4.96 V -> approximately 4.6 V
```

This indicates that the motor load affects the Arduino power rail.

The actual startup voltage may fall lower than the multimeter reading because the multimeter averages fast transients.

---

# 13. Hardware Verification Strategy

## 13.1 LED Substitution Test

Replace the motor with:

```text
LED + resistor
```

Connection:

```text
5 V
 |
220 Ohm to 1 kOhm
 |
LED
 |
Collector
Emitter
 |
GND
```

Expected behavior:

```text
fan_cooling OFF -> LED OFF
fan_cooling ON  -> LED ON
```

---

## 13.2 Separating Software and Hardware Problems

If the LED test works:

```text
Control software probably works
D8 probably works
Transistor switching probably works
```

The remaining problem is more likely related to:

* Motor current
* Motor noise
* Power supply
* Flyback diode
* Wiring resistance

---

## 13.3 Recommended Verification Order

1. Test the Arduino output without the motor.
2. Test the transistor using an LED load.
3. Confirm the transistor pinout.
4. Confirm flyback diode direction.
5. Measure D8.
6. Measure VBE.
7. Measure VCE.
8. Connect the motor.
9. Measure the 5 V rail.
10. Evaluate the need for a separate motor supply.

---

# 14. Recommended Circuit Improvements

## 14.1 Separate Motor Supply

Use a separate supply for the motor while keeping common ground.

This prevents motor current from flowing through the Arduino 5 V supply path.

---

## 14.2 Logic-Level MOSFET

A logic-level N-channel MOSFET may be more suitable than a small BJT for higher motor current.

Advantages:

* Very low ON resistance
* Low voltage drop
* Almost no steady-state gate current
* Lower power dissipation

Typical control arrangement:

```text
Arduino output
    |
100 Ohm gate resistor
    |
MOSFET gate

Gate
 |
10 kOhm pull-down
 |
GND
```

---

## 14.3 Improved Decoupling

Recommended capacitors:

```text
104 ceramic capacitor
-> Across motor terminals

220-1000 uF electrolytic capacitor
-> Near motor supply input

0.1 uF ceramic capacitor
-> Near Arduino supply pins
```

---

## 14.4 Better Wiring Layout

Recommended practices:

* Keep motor-current wires short
* Keep sensor wires away from motor wires
* Use thicker wires for motor current
* Avoid routing motor current through sensitive ground paths
* Place the flyback diode close to the motor or switching device
* Place bulk capacitors close to the motor supply connection

---

## 14.5 Future Current Sensing

A future version may include current sensing using:

* A small shunt resistor
* A current-sense amplifier
* An ACS712-type current sensor
* A dedicated motor-driver IC

This would allow the software to detect:

* Startup current
* Overcurrent
* Stalled motor
* Motor disconnection

---

# 15. Safety Notes

## 15.1 Low-Voltage Prototype Safety

Even in a 5 V circuit:

* Avoid short circuits
* Disconnect power before changing wiring
* Stop immediately if a component becomes hot
* Check capacitor polarity
* Use the correct multimeter range
* Do not connect an ammeter directly across a voltage source

---

## 15.2 Mains Voltage Warning

This breadboard circuit must not be directly adapted to:

```text
100 V AC
220 V AC
230 V AC
```

A point can have dangerous voltage even when no current is currently flowing.

Touching it may complete a current path through the body.

Mains-voltage work requires proper:

* Isolation
* Fusing
* Enclosure
* Grounding
* Insulation
* Protective equipment
* Qualified supervision

---

## 15.3 Capacitor Polarity

Electrolytic capacitors are polarized.

Typical identification:

```text
Long lead  -> Positive
Short lead -> Negative
Stripe with minus signs -> Negative
```

Connection:

```text
Positive -> 5 V
Negative -> GND
```

Ceramic capacitors such as `104` normally have no polarity.

---

## 15.4 Current Measurement Safety

When measuring current:

1. Move the multimeter lead to the correct current socket.
2. Start with the highest current range.
3. Connect the meter in series.
4. Do not connect the current meter directly across 5 V and GND.
5. Return the probe to the voltage socket after measurement.

---

# 16. Lessons Learned

* The Arduino output pin should control a load, not directly power it.
* A transistor separates the control path from the motor-current path.
* The base resistor protects the Arduino and transistor.
* The flyback diode protects the circuit from inductive voltage spikes.
* The 104 capacitor suppresses high-frequency noise.
* A large electrolytic capacitor supports short current transients.
* A capacitor cannot replace an inadequate power supply.
* VBE helps confirm base-emitter conduction.
* VCE helps identify transistor OFF and saturation states.
* Motor startup current can be much larger than running current.
* Power-supply problems can appear as serial communication problems.
* A multimeter may not capture short voltage transients.
* An oscilloscope can reveal startup behavior and electrical noise.
* External motor power requires a common ground.
* LED substitution is useful for separating control problems from power problems.
* Measured values should be recorded before modifying the circuit.
* Low-voltage breadboard circuits must not be directly adapted to mains voltage.
