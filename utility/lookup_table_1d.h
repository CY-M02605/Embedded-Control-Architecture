/**
 * @file lookup_table_1d.h
 * @brief Provides a generic one-dimensional lookup table with linaer interpolation and boundary handling
 * @date 2026-6-30
 */

#ifndef LOOKUP_TABLE_1D_H
#define LOOKUP_TABLE_1D_H

#include <cstddef>

namespace utility {

template <typename T>
class LookupTable1D {
public:
    struct Point {
        T input;
        T output;
    };

    LookupTable1D(const Point* points, std::size_t size):
    points_(points), size_(size) {}

    T LookupTable(T input) const {
        if (input <= points_[0].input) {
            return points_[0].output;
        }

        if (input > points_[size_ - 1].input) {
            return points_[size_ - 1].output;
        }

        for (int i = 0; i < size_ - 1; ++i) {
            Point lower = points_[i];
            Point upper = points_[i + 1];

            if (input > lower.input && input <= upper.input) {
                T ratio = (input - lower.input) / (upper.input - lower.input);
                T output = ratio * (upper.output - lower.output) + lower.output;
                return output;
            }
        }

        // This path should be unreachable if the table is valid and sorted.
        return points_[size_ - 1].output;
    }

private:
    const Point* points_;
    std::size_t size_;
};
}   // namespace utility

#endif
