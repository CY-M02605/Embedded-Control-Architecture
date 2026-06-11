// wlCoolingFanControl.h
#ifndef WL_COOLING_FAN_CONTROL_C_H
#define WL_COOLING_FAN_CONTROL_C_H

typedef float Float32;
typedef int   Int32;

// 输入：coolantTemp（冷却液温度℃）、engineLoad（发动机负载%）
// 输出：fanDuty（风扇占空比%）、overheatFlag（过热标志 0/1）
extern void wlCoolingFanControl(Float32* coolantTemp, Float32* engineLoad, 
                                 Float32* fanDuty, Int32* overheatFlag);
extern void INIT_wlCoolingFanControl(void);

#endif