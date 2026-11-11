#pragma once

// std
#include <cstddef>
#include <span>

// ers
#include <erslib/core/hashing/base.hpp>


// Details ---------------------------------------------------------------------

#include "impl/rapid.hpp"


// Implementation --------------------------------------------------------------

// Algos usage

namespace ers::impl {
    struct rapid_hash_policy {};
}


template<>
struct ers::impl::hashing::backend<ers::impl::rapid_hash_policy> {
    static constexpr size_t process_raw_bytes(std::span<const std::byte> what, size_t seed) noexcept {
        return rapidhash_micro(what.data(), what.size(), seed, rapid_secret);
    }
};


// Declaration -----------------------------------------------------------------

namespace ers::impl {
    template<typename T>
    using RapidHash = THashBase<T, rapid_hash_policy>;
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::rapid_hash_policy;
    using impl::RapidHash;
}
