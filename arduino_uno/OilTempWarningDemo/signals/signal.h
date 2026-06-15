/**
 * @file signal.h
 * @brief Declares the Signal for Arduino UNO.
 * @date 2026-06-11
 */

#ifndef SIGNAL_H
#define SIGNAL_H

namespace signals {

enum class OnOffStatus {
    OFF,
    ON
};

enum class ValidityStatus {
    VALID,
    INVALID
};

template <typename T>
class Signal {
    public:
        Signal()
            : value_(), validity_(signals::ValidityStatus::VALID) {}

        Signal(T value, ValidityStatus validity) 
            : value_(value), validity_(signals::ValidityStatus::VALID) {}

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

        bool IsValid() const {
            return validity_ == ValidityStatus::VALID;
        }

        void Set(T value, ValidityStatus validity) {
            value_ = value;
            validity_ = validity;
        }

    private:
        T value_;
        ValidityStatus validity_;
        
};

using BoolSignal = Signal<bool>;
using FloatSignal = Signal<float>;
using IntSignal = Signal<int>; 

}

#endif
