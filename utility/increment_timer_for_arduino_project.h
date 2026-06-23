/**
 * @file increment_timer_for_arduino_project.h
 * @brief another IncrementTimer just for arduino_project unit test
 * @date 2026-06-23
 */

#ifndef INCREMENT_TIMER_FOR_ARDUINO_PROJECT_H
#define INCREMENT_TIMER_FOR_ARDUINO_PROJECT_H

namespace utility {
class IncrementTimerForAP {
    public:
        explicit IncrementTimerForAP(
            unsigned long interval_ms,
            unsigned long previous_ms
        ):interval_ms_(interval_ms),
        previous_ms_(previous_ms),
        output_(false) {};

        void Update(unsigned long current_ms) {
            if (current_ms - previous_ms_ >= interval_ms_) {
                previous_ms_ = current_ms;
                output_ = true;
            } else {
                output_ = false;
            }
        }

        bool OutputRef() {
            return output_;
        }

    private:
        unsigned long interval_ms_;
        unsigned long previous_ms_;
        bool output_;
};
}

#endif
