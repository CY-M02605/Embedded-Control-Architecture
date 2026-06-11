#ifndef INCREMENT_TIMER_H
#define INCREMENT_TIMER_H

namespace utility {
class IncrementTimer {
    public:
        struct Config {
            int threshold;
        };

        // explicit: prevents implicit type conversion from Config to IncrementTimer
        explicit IncrementTimer(const Config& config)
            :config_(config), count_(0) { }

        void Update() {
            if (count_ < config_.threshold) {
                ++count_;
            }
        }

        bool IsTimeUp() {
            if (count_ >= config_.threshold) {
                return true;
            } else {
                return false;
            }
        }

        void Clear() {
            count_ = 0;
        }

    private:
        const utility::IncrementTimer::Config& config_;
        int count_;
};
}

#endif
