/**
 * @file hysteresis.h
 * @brief Provide a hysteresis utility for processing input signals
 * @date 2026-06-11
 */

#ifndef HYSTERESIS_H
#define HYSTERESIS_H

#include "signal.h"

namespace utility {
template <typename T>
class Hysteresis {
    public:
        struct Config {
            T low_threshold;
            T high_threshold;
        };

        // A constructor parameter in parentheses
        explicit Hysteresis (Config config):
            // Do not bind a reference member to a constructor parameter passed by value,
            // because the parameter is destroyed when the constructor returns.
            config_(config),
            status_(signals::OnOffStatus::OFF) {}

        void Update(T value) {
            if (status_ == signals::OnOffStatus::OFF) {
                if (value >= config_.high_threshold) {
                    status_ = signals::OnOffStatus::ON;
                }
            } else {
                if (value <= config_.low_threshold) {
                    status_ = signals::OnOffStatus::OFF;
                }
            }
        }


    private:
        // Create a reference member.
        const Config config_;
        signals::OnOffStatus& status_;

};
}   // namespace utility

#endif
