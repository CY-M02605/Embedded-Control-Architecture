/**
 * @file signal.h
 * @brief Defines reusable typed signals with value and validity information.
 * @date 2026-06-11
 */

#ifndef SIGNAL_H
#define SIGNAL_H

namespace signals {

enum class ValidityStatus {
    VALID,
    INVALID
};

enum class OnOffStatus {
    OFF,
    ON
};

template<typename T>
class Signal {
    public:
        Siganl():value_(T()), validity_(ValidityStatus::VALID) {}
        Siganl(T value, ValidityStatus validity)
            : value_(value),
            validity_(validity) {}

        void SetValue(T value) {
            value_ = value;
        }

        T GetValue() const {
            return value_;
        }

        void SetValidity(ValidityStatus validity) {
            validity_ = validity;
        }

        ValidityStatus GetValidity() const {
            return validity_;
        }

        void Set(T value, ValidityStatus validity) {
            value_ = value;
            validity_ = validity;
        }

        bool IsValid() const {
            return validity_ == signals::ValidityStatus::VALID;
        }

    private:
        T value_;
        ValidityStatus validity_;
};

typedef Signal<int> IntSignal;
typedef Signal<float> FloatSignal;
typedef Signal<bool> BoolSignal;

typedef Signal<OnOffStatus> OnOffSignal;
typedef Signal<ValidityStatus> ValiditySignal;
}

#endif
