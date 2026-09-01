# EngineOverheatProtection module for Arduino

## Purpose

The `EngineOverHeatProtection` module monitors engine running status and hydraulic oil temperature. It activates overheat protection when the temperature above the configured high threshold for the cofigured duration.

## Inputs

- `oil_temp`
- `is_engine_running`
- `clear_fault_request`

Each input is represented by a typed signal with a value and a validity status.

## Outputs

- `is_overheat_protected`
- `torque_limit`
- `fan_request`
- `state`
- `fault_reason`

## Main States

- `STOP`
- `IDLE`
- `COUNTING`
- `PROTECTED`
- `AFTER_RUN_COOLING`
- `FAULT`


## Testing

This module is tested by PC-based assert tests and GitHub Actions.

Test path:

```text
tests/modules/engine_overheat_protection_Arduino/assert_based_tests.cpp
```