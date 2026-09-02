/**
 * @file rate_of_change_checker.h
 * @brief Provides a rate change checker utility for each module in this project
 * @date 2026-09-02
 */

#ifndef RATE_OF_CHANGE_CHECKER_H
#define RATE_OF_CHANGE_CHECKER_H

namespace utility {
class RateOfChangeChecker {
public:
    struct Config
    {
        float max_rate_of_change;
    };
    
    explicit RateOfChangeChecker(Config config)
        : config_(config), 
        has_previous_value_(false),
        previous_value_(0.0f) {}

    bool CheckRateOfChange(float current_value, float timer_interval) {

        if (timer_interval <= 0) {
            previous_value_ = current_value;
            return false;
        }

        if (!has_previous_value_) {
            previous_value_ = current_value;
            has_previous_value_ = true;
            return false;
        }

        const float rate_of_change_ = (current_value - previous_value_) / timer_interval;
        previous_value_ = current_value;
        if (rate_of_change_ >= config_.max_rate_of_change) {
            return true;
        } else {
            return false;
        }
    }

    void Reset() {
        previous_value_ = 0.0f;
        has_previous_value_ = false;
    }

private:
    const Config config_;
    float previous_value_;
    bool has_previous_value_;
};
}

#endif
