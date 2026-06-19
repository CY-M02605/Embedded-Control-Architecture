/**
 * @file increment_timer.h
 * @brief IncrementTimer for arduino_uno in utility
 * @date 11.06.2026
 */

#ifndef INCREMENT_TIMER_H
#define INCREMENT_TIMER_H

#include <Arduino.h>

namespace utility {
class IncrementTimer {
    public:
         struct Config {
            int time_threshold;
         };

        // explicit IncrementTimer(
        //     const utility::IncrementTimer::Config& config,
        //  ):config_(config), count_(0) {}

        explicit IncrementTimer(unsigned long interval_ms)
            : interval_ms_(interval_ms),
            previous_ms_(0),
            initialized_(false) {}

        bool IsExpired() {
            unsigned long current_ms = millis();

            if (!initialized_) {
                previous_ms_ = current_ms;
                initialized_ = true;
                return false;
            }

            if (current_ms - previous_ms_ >= interval_ms_) {
                previous_ms_ = current_ms;
                return true;
            }

            return false;
        }

        // void Update() {
        // if (count_ < config_.time_threshold) {
        //     ++ count_;
        // }
        // }

        // bool IsTimeUp() const{
        // if (count < config_.time_threshold) {
        //     return false;
        // } else {
        //     return true;
        // }
        // }

        // void Clear() {
        // count_ = 0;
        // }

        void Reset() {
            previous_ms_ = millis();
            initialized_ = true;
        }

        void SetInterval(unsigned long interval_ms) {
            interval_ms_ = interval_ms;
        }

        unsigned long GetInterval() const {
            return interval_ms_;
        }

    private:
        // const utility::IncrementTimer::Config& config_;
        // int count_;

        unsigned long interval_ms_;
        unsigned long previous_ms_;
        bool initialized_;

};
}

#endif
