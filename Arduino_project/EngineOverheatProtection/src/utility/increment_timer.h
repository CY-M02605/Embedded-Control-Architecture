/**
 * @file increment_timer.h
 * @brief provide a increment timer to EngineOverheatProtection for counting the time of protection status
 * @date 2026-07-24
 */

#ifndef INCREMENT_TIMER_H
#define INCREMENT_TIMER_H

#include <cstddef>

namespace utility {
class IncrementTimer {
    public:
        struct Config {
            std::size_t threshold_time;
        };

        explicit IncrementTimer(const Config config):
            config_(config), count_(0) {}

        void Update() {
            if (count_ < config_.threshold_time) {
                count_++;
            }
        }

        bool IsTimeUp() const {
            if (count_ == config_.threshold_time) {
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
        std::size_t count_;

};
}

#endif
