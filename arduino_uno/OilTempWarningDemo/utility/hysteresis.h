/**
 * @file hysteresis.h
 * @brief Declares the Hysteresis for Arduino UNO.
 * @date 2026-06-11
 */

#ifndef HYSTERESIS_H
#define HYSTERESIS_H

namespace utility {
class Hysteresis {
    public:
        struct Config {
            float low_threshold;
            float high_threshold;
        };

        // enum class OnOffStatus {
        //     OFF,
        //     ON
        // };

        explicit Hysteresis(
            utility::Hysteresis::Config config
        ):config_(config),
        status_(false) {}

        bool Update(float value) {
            switch (status_)
            {
            case false:
                if (value >= config_.high_threshold) {
                    status_ = true;
                    return status_;
                }
            
            case true:
            if (value <= config_.low_threshold) {
                status_ = false;
                return status_;
            }
            }
        }

        bool GetStatus() const {
            return status_;
        }

        bool IsTrue() const {
            return status_ == true;
        }

        void Reset(bool initial_status = false) {
            status_ = initial_status;
        }

    private:
        utility::Hysteresis::Config config_;
        bool status_;
};
}

#endif
