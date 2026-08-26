/**
 * @file lookup_table_1d.h
 * @brief Provides a generic one-dimensional lookup table with linaer interpolation and boundary handling for Arduino hardware system
 * @date 2026-07-23
 */

#ifndef LOOKUP_TABLE_1D_H
#define LOOKUP_TABLE_1D_H

// #include <cstddef>
#include <stddef.h>

namespace utility {

template <typename T>
class LookupTable1D {
    public:
        struct Points {
            T input;
            T output;
        };

    LookupTable1D(const Points* points, size_t size)
     : points_(points), size_(size) {}

    T LookupTable(T input) const {
        if (input < points_[0].input) {
            return points_[0].output;
        } else if (input >= points_[size_ - 1].input) {
            return points_[size_ - 1].output;
        } else {
            size_t count;
            for (count = 0; count < size_ - 1; ++count) {
                if (input >= points_[count].input && input <= points_[count + 1].input) {
                    T lower_input = points_[count].input;
                    T upper_input = points_[count + 1].input;

                    T lower_output = points_[count].output;
                    T upper_output = points_[count + 1].output;

                    T ratio = (upper_output - lower_output) / (upper_input - lower_input);
                    return ratio * (input - lower_input) + lower_output;
                }
            }
        }

        return points_[size_ - 1].output;
    }

    private:
        const Points* points_;
        size_t size_;
};
}

#endif
