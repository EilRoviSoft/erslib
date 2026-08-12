#pragma once

// ers
#include <erslib/core/hashing/base.hpp>


// Implementation

#include "impl/rapid.hpp"


// Algos usage

namespace ers::impl {
    struct rapid_hash_policy {};
}


template<>
struct ers::hashing::backend<ers::impl::rapid_hash_policy> {
    static constexpr size_t process_raw_bytes(std::span<const std::byte> what, size_t seed) noexcept {
        return impl::hashing::rapidhash_micro(what.data(), what.size(), seed, impl::hashing::rapid_secret);
    }
};


// Declaration

namespace ers::impl {
    template<typename T>
    using RapidHash = THashBase<T, rapid_hash_policy>;
}

namespace ers {
    using impl::rapid_hash_policy;
    using impl::RapidHash;
}
