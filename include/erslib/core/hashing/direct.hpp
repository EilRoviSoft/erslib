#pragma once

// ers
#include <erslib/core/hashing/base.hpp>


// Algos usage

namespace ers::impl {
    struct direct_hash_policy {};
}


template<>
struct ers::hashing::backend<ers::impl::direct_hash_policy> {
    template<std::integral T>
    static constexpr size_t process_value(const T& value, size_t /*seed*/) noexcept {
        return static_cast<size_t>(value);
    }
};


// Declaration

namespace ers::impl {
    // Useful when you already provide hashed values.
    template<typename T>
    using DirectHash = THashBase<T, direct_hash_policy>;
}

namespace ers {
    using impl::direct_hash_policy;
    using impl::DirectHash;
}
