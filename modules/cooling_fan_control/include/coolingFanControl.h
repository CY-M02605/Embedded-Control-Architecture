// wlCoolingFanControl.h
#ifndef COOLING_FAN_CONTROL_C_H
#define COOLING_FAN_CONTROL_C_H

typedef float Float32;
typedef int   Int32;

// input: coolantTemp (celsius degree)、engineLoad (%)
// output: fanDuty (%)、overheatFlag (0/1)
extern void coolingFanControl(Float32* coolantTemp, Float32* engineLoad, 
                                 Float32* fanDuty, Int32* overheatFlag);
extern void INIT_coolingFanControl(void);

#endif