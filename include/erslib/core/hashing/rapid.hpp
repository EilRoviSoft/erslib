#pragma once

// ers
#include <erslib/core/hashing/base.hpp>


// Implementation

#include "rapid-impl.hpp"


// Algos usage

namespace ers {
    struct rapid_hash_policy {};
}


template<>
struct ers::hashing::backend<ers::rapid_hash_policy> {
    static constexpr size_t process_raw_bytes(std::span<const std::byte> what, size_t seed) noexcept {
        return internal::rapidhash_micro(what.data(), what.size(), seed, internal::rapid_secret);
    }
};


// Declaration

namespace ers {
    template<typename T>
    using RapidHash = THashBase<T, rapid_hash_policy>;
}
