#pragma once

// ers
#include <erslib/core/hashing/base.hpp>


namespace ers {
    struct direct_hash_policy {};
}


template<>
struct ers::hashing::backend<ers::direct_hash_policy> {
    template<std::integral T>
    static constexpr size_t process_value(const T& value, size_t /*seed*/) noexcept {
        return static_cast<size_t>(value);
    }
};


// Declaration

namespace ers {
    // Useful when you already provide hashed values.
    template<typename T>
    using DirectHash = THashBase<T, direct_hash_policy>;
}
