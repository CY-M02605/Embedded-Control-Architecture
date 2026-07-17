// wlVehicleSpeed.c
#include "wlVehicleSpeed.h"

void wlVehicleSpeed(Float32* engineRpm, Int32* gearRatio, Float32* vehicleSpeed) {
    // 简化公式：车速 = 转速 / (传动比 * 100)
    if (*gearRatio != 0) {
        *vehicleSpeed = *engineRpm / (*gearRatio * 100.0f);
    } else {
        *vehicleSpeed = 0.0f;
    }
}

void INIT_wlVehicleSpeed(void) {
    // 无需初始化
}