#pragma once

// ers
#include <erslib/hashing/base.hpp>


// Implementation

#include "rapid-impl.hpp"


// Algos usage

namespace ers::hashing {
    struct rapid_policy {};
}


template<>
struct ers::hashing::backend<ers::hashing::rapid_policy> {
    static constexpr size_t process_raw_bytes(std::span<const std::byte> what, size_t seed) noexcept {
        return internal::rapidhash_micro(what.data(), what.size(), seed, internal::rapid_secret);
    }
};


// Declaration

namespace ers {
    template<typename T>
    using RapidHash = THashBase<T, hashing::rapid_policy>;
}
