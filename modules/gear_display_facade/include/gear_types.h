#ifndef GEAR_TYPES_H
#define GEAR_TYPES_H

// ---- Domain enum types (like TmState, TravelDirection in company code) ----

// input variable 1: ac_gear_position (int)
enum GearPosition {
    NEUTRAL,
    FORWARD_1,
    FORWARD_2,
    FORWARD_3,
    REVERSE_1,
    REVERSE_2
};

// input variable 2: ac_is_eco_mode (bool)
enum DriveMode {
    ECO,
    POWER
};

#endif
