/**
 * @file test_increment_timer.cpp
 * @brief IncrementTimerTest for increment timer unit test in Arduino project
 * @date 2026-06-23
 */

// because the Arduino function millis() in increment_timer.h in Arduino_project
// I rewrote another increment_timer_for_arduino_project.h in outside utility folder for unit test
// #include "utility/increment_timer.h"
#include "increment_timer_for_arduino_project.h"

#include <cassert>

int main() {
    utility::IncrementTimerForAP increment_timer(100, 0);

    increment_timer.Update(0UL);
    assert(increment_timer.GetOutPut() == false);

    increment_timer.Update(99);
    assert(increment_timer.GetOutPut() == false);

    increment_timer.Update(100);
    assert(increment_timer.GetOutPut() == true);

    increment_timer.Update(101);
    assert(increment_timer.GetOutPut() == false);

    increment_timer.Update(199);
    assert(increment_timer.GetOutPut() == false);

    increment_timer.Update(200);
    assert(increment_timer.GetOutPut() == true);

    increment_timer.Update(201);
    assert(increment_timer.GetOutPut() == false);    
    
    return 0;
}