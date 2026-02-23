#pragma once

// ers
#include <erslib/hashing/base.hpp>


namespace ers::hashing {
    struct direct_policy {};
}


template<>
struct ers::hashing::backend<ers::hashing::direct_policy> {
    template<std::integral T>
    static constexpr size_t process_value(const T& value, size_t /*seed*/) noexcept {
        return static_cast<size_t>(value);
    }
};


// Declaration

namespace ers {
    // Useful when you already provide hashed values.
    template<typename T>
    using DirectHash = THashBase<T, hashing::direct_policy>;
}
