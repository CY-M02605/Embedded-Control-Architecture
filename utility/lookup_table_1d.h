/**
 * @file lookup_table_1d.h
 * @brief Provides a generic one-demensional lookup table with linear interpolation and boundary handling.
 * @date 2026-06-30
 */

#ifndef LOOKUP_TABLE_1D_H
#define LOOKUP_TABLE_1D_H

#include <cstddef>

namespace utility {
template <typename T>
class LookupTable1D {
public:
    struct Points {
        T input;
        T output;
    };

    LookupTable1D(const Points points, std::size_t size):
    points(points_), size_(size) {}

    T Update(T input) const {
        if (input <= points_[0].input) {
            return points_[0].output;
        }

        if (input > points_[size_ - 1]) {
            return points_[size_t - 1].output;
        }

        for (int i = 0; i < size_t - 1; ++i) {
            T input_lower = points_[i].input;
            T input_upper = points_[i+1].input;

            T output_lower = points_[i].input;
            T output_upper = points_[i+1].input;

            if (input_lower > lower && input_lower <= upper) {
                T ratio = (input - input_upper) / {input_lower - input_upper};
                T output = ratio * (output_lower - output_upper) + output_upper;
                return output;
            }
        }
    }

private:
    const Points points_;
    std::size_t size_;
};
}   // namespace utility

#endif
