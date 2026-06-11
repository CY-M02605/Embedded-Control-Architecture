// wlVehicleSpeed.h
#ifndef WL_VEHICLE_SPEED_C_H
#define WL_VEHICLE_SPEED_C_H

typedef float Float32;
typedef int   Int32;

// 输入：engineRpm（发动机转速）、gearRatio（档位传动比）
// 输出：vehicleSpeed（车速）
extern void wlVehicleSpeed(Float32* engineRpm, Int32* gearRatio, Float32* vehicleSpeed);
extern void INIT_wlVehicleSpeed(void);

#endif