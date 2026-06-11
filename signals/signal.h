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

template <typename T>
class Signal {
    public:
        // Default constructor
        Signal() : value_(T()), validity_(VALID) {}

        // Parameterized constructor
        Signal(T value, ValidityStatus validity)
            :value_(value), validity_(validity) {}

        void SetValue(T value) { value_ = value; }
        void SetValidity(ValidityStatus validity) { validity_ = validity; }
        void Set(T value, ValidityStatus validity) { value_ = value; validity_ = validity; }

        T GetValue() const { return value_; }
        ValidityStatus GetValidity() const { return validity_; }
        bool IsValid() const { return validity_ == ValidityStatus::VALID; }
    
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
