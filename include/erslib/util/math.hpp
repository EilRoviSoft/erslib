#pragma once

// std
#include <cmath>
#include <concepts>
#include <limits>

namespace ers::util {
    template<std::floating_point T>
    bool equals(T lhs, T rhs) {
        return std::fabs(lhs - rhs) < std::numeric_limits<T>::epsilon();
    }
}
