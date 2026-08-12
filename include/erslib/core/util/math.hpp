#pragma once

// std
#include <cmath>
#include <concepts>
#include <limits>


namespace ers::impl::util {
    template<std::floating_point T>
    bool equals(T lhs, T rhs) {
        return std::fabs(lhs - rhs) < std::numeric_limits<T>::epsilon();
    }


    template<typename T>
    constexpr int log10_ceil(T x) {
        return x < 10 ? 1 : 1 + log10_ceil(x / 10);
    }
}


// Exports

namespace ers {
    namespace util = impl::util;
}
