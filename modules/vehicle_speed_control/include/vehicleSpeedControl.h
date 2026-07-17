// wlVehicleSpeed.h
#ifndef VEHICLESPEEDCONTROL_H
#define VEHICLESPEEDCONTROL_H

typedef float Float32;
typedef int   Int32;

// input: engineRpm, gearRatio
// output: vehicleSpeed
extern void vehicleSpeedContol(Float32* engineRpm, Int32* gearRatio, Float32* vehicleSpeed);
extern void INIT_vehicleSpeedControl(void);

#endif