/**
 * @file torque_lookup_table.h
 * @brief Provide a engine torque decreasing utility for emergency
 * @date 2026-06-30
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

    LookupTable1D(const Point* points, std::size_t size)
        : points_(points),
          size_(size) {
    }

    T Lookup(T input) const {
        if (input <= points_[0].input) {
            return points_[0].output;
        }

        if (input >= points_[size_ - 1].input) {
            return points_[size_ - 1].output;
        }

        for (std::size_t i = 0; i < size_ - 1; ++i) {
            const Point& lower = points_[i];
            const Point& upper = points_[i + 1];

            if (input >= lower.input && input <= upper.input) {
                const T ratio =
                    (input - lower.input) /
                    (upper.input - lower.input);

                return lower.output +
                       ratio * (upper.output - lower.output);
            }
        }

        return points_[size_ - 1].output;
    }

private:
    const Point* points_;
    std::size_t size_;
};

}  // namespace utility

#endif
