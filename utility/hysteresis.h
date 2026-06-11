#ifndef HYSTERESIS_H
#define HYSTERESIS_H

namespace utility {
class Hysteresis {
    public:
        struct Config {
            float high_threshold;
            float low_threshold;
        };

        explicit Hysteresis(const Config& config)
            : config_(config), 
            //   state_(false),
              status_(OFF) {}

        enum STATUS {
            ON,
            OFF
        };

        void Update(float value) {
            switch (status_)
            {
            case ON:
                if (value <= config_.low_threshold) {
                    status_ = OFF;
                }
                break;
            
            case OFF:
                if (value >= config_.high_threshold) {
                    status_ = ON;
                }
                break;
            }
        }

        // bool Update(float value) {
        //     if (state_) {
        //         if (value <= config_.low_threshold) {
        //             state_ = false;
        //         }
        //     } else {
        //         if (value >= config_.high_threshold) {
        //             state_ = true;
        //         }
        //     }
        // }

        // Alternative: return bool
        // bool GetState() {
        //     return status_ == ON;
        // }
        // Current: return STATUS enum
        STATUS GetState() {
            return status_;
        }

    private:
        const utility::Hysteresis::Config& config_;
        // bool state_;
        STATUS status_;
};
}

#endif
