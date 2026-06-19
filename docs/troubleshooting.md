# Troubleshooting Notes

This document records the problems I encountered while learning Arduino UNO R3, together with the causes and solutions.

## 1. `analogWrite()` and `digitalWrite()`

### Problem

I was confused about the difference between `digitalWrite()` and `analogWrite()`.

### Cause

`digitalWrite()` only outputs `HIGH` or `LOW`.

`analogWrite()` outputs PWM signals on supported pins, such as D3, D5, D6, D9, D10, and D11 on Arduino UNO.

### Solution

Use `digitalWrite()` for simple ON/OFF control.

Use `analogWrite()` for brightness control, motor speed control, or buzzer tone experiments.

## 2. RGB LED value became negative

### Problem

The RGB LED brightness variable, such as `redValue`, sometimes became negative.

### Cause

The variable was decreased repeatedly inside a loop:

```cpp
redValue -= 1;
```

If the loop runs too many times or the value is not reset, the value can go below 0.

### Solution

Use the loop counter directly:

```cpp
analogWrite(RED, 255 - count);
analogWrite(GREEN, count);
```

Or limit the value:

```cpp
redValue = constrain(redValue, 0, 255);
```

## 3. LCD showed nothing

### Problem

The LCD backlight was on, but no characters were displayed.

### Possible Causes

* LCD contrast was not adjusted correctly.
* LCD wiring did not match the `LiquidCrystal` constructor.
* The RW pin was not connected to GND.
* The wrong constructor parameters were used.

### Solution

Check the wiring and test with a minimal program:

```cpp
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 10, 5, 4, 3, 2);

void setup() {
  lcd.begin(16, 2);
  lcd.print("Hello World");
}

void loop() {
}
```

## 4. NPN and PNP Sensor Output

### Problem

I was confused about why NPN is called sink output and PNP is called source output.

### Cause

The terms source and sink are defined from the load's point of view, not from the transistor's internal structure.

### Understanding

NPN output pulls the load side down to GND.

PNP output supplies voltage to the load side.

### Summary

* NPN: output becomes 0V when active.
* PNP: output becomes +24V when active.
