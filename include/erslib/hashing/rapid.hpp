#pragma once

// ers
#include <erslib/hashing/base.hpp>

// Implementation

#include "rapid-impl.hpp"

// Algos usage

namespace ers::hashing {
    struct rapidhash_policy {};
}

template<>
struct ers::internal::backend<ers::hashing::rapidhash_policy> {
    static constexpr size_t process(
        const std::byte* data,
        size_t size,
        size_t seed
    ) noexcept {
        return rapidhash_micro_with_seed(data, size, seed);
    }
};

// Declaration

namespace ers {
    template<typename T>
    using RapidHash = THashBase<T, hashing::rapidhash_policy>;
}
