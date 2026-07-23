/**
 * @file signal.h
 * @brief Signal in EngineOverheatProtection module for Arduino hardware system
 * @date 2026-07-23
 */

#ifndef SIGNAL_H
#define SIGNAL_H

enum class OnOffStatus {
    ON,
    OFF
};

enum class ValidityStatus {
    VALID,
    INVALID
};

namespace signal {
template <typename T>
class Signal {
    public:
        signal()
            : value_(), validity_(ValidityStatus::VALID) {};

        Signal(T value, ValidityStatus validity)
            : value_(value), validity_(validity) {}

        void SetValue(T value) {
            value_ = value;
        }

        void SetValidity(ValidityStatus validity) {
            validity_ = validity;
        }

        void Set(T value, ValidityStatus validity) {
            value_ = value;
            validity_ = validity;
        }

        T GetValue() const {
            return value_;
        }

        ValidityStatus GetValidity() const {
            return validity_;
        }

        bool IsValid() const {
            return validity_ == ValidityStatus::VALID;
        }

    private:
        T value_;
        ValidityStatus validity_;

};

typedef Signal<OnOffStatus> OnOffSignal; 
typedef Signal<ValidityStatus> ValiditySignal;
typedef Signal<int> IntSignal;
typedef Signal<float> FloatSignal;
typedef Signal<bool> BoolSignal;

}

#endif
