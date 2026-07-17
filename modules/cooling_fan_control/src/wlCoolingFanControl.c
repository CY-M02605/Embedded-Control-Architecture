// wlCoolingFanControl.c
#include "wlCoolingFanControl.h"

void wlCoolingFanControl(Float32* coolantTemp, Float32* engineLoad,
                          Float32* fanDuty, Int32* overheatFlag) {
    // 基础占空比 = 温度映射
    Float32 base = (*coolantTemp - 70.0f) * 2.0f;  // 70℃以下=0%, 120℃=100%
    if (base < 0.0f) base = 0.0f;
    if (base > 100.0f) base = 100.0f;

    // 高负载时额外加 20%
    if (*engineLoad > 80.0f) {
        base += 20.0f;
        if (base > 100.0f) base = 100.0f;
    }

    *fanDuty = base;
    *overheatFlag = (*coolantTemp >= 110.0f) ? 1 : 0;
}

void INIT_wlCoolingFanControl(void) {}