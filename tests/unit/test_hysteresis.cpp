/**
 * @file test_hysteresis.cpp
 * @brief HysteresisTest for hysteresis in  unit test 
 * @date 2026-06-22
 */

#include <cassert>
#include <iostream>

#include "utility/hysteresis.h"

int main() {
    utility::Hysteresis hysteresis({
        80.0f,
        85.0f
    });

    // initial state
    hysteresis.Update(70.0f);
    assert(hysteresis.GetStatus() == false);

    hysteresis.Update(75.0f);
    assert(hysteresis.GetStatus() == false);

    hysteresis.Update(80.0f);
    assert(hysteresis.GetStatus() == false);

    hysteresis.Update(84.9f);
    assert(hysteresis.GetStatus() == false);

    hysteresis.Update(85.0f);
    assert(hysteresis.GetStatus() == true);

    hysteresis.Update(85.1f);
    assert(hysteresis.GetStatus() == true);

    hysteresis.Update(90.0f);
    assert(hysteresis.GetStatus() == true);

    hysteresis.Update(85.0f);
    assert(hysteresis.GetStatus() == true);

    hysteresis.Update(80.1f);
    assert(hysteresis.GetStatus() == true);

    hysteresis.Update(80.0f);
    assert(hysteresis.GetStatus() == false);

    hysteresis.Update(79.9f);
    assert(hysteresis.GetStatus() == true);

    hysteresis.Update(75.0f);
    assert(hysteresis.GetStatus() == false);

    std::cout << "All Hysteresis teste passed.\n";

    return 0;
}
