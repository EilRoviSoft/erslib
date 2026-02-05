#pragma once

// std
#include <optional>

namespace ers {
    template<typename T>
    using ref = std::reference_wrapper<T>;

    template<typename T>
    using cref = std::reference_wrapper<const T>;
}
