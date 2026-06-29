/**
 * @file increment_timer.h
 * @brief Provides a timer utility for managing timed state transitions.
 * @date 2026-06-11
 */

#ifndef INCREMENT_TIMER_H
#define INCREMENT_TIMER_H

namespace utility {
class IncrementTimer {
    public:
        struct Config {
            int threshold_timer;
        };

        // explicit: prevents implicit type conversion from Config to IncrementTimer
        explicit IncrementTimer(Config config):
            config_(config),
            count_(0) {}

        void Update() {
            if (count_ < config_.threshold_timer) {
                ++count_;
            }
        }

        bool IsTimeUp() const {
            if (count_ >= config_.threshold_timer) {
                return true;
            } else {
                return false;
            }
        }

        void Clear() {
            count_ = 0;
        }

    private:
        const Config config_;
        int count_;
};
}   // namespace utility

#endif
