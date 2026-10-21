#pragma once

// std
#include <cstddef>
#include <span>

// ers
#include <erslib/core/hashing/base.hpp>


// Details ---------------------------------------------------------------------

#include "impl/fnv.hpp"


// Implementation --------------------------------------------------------------

// Algos usage

namespace ers::impl {
    struct fnv_1a_hash_policy {};
}


template<>
struct ers::hashing::backend<ers::impl::fnv_1a_hash_policy> {
    static constexpr size_t process_raw_bytes(std::span<const std::byte> what, size_t seed) noexcept {
        return impl::hashing::fnv_1a(what.data(), what.size(), seed);
    }
};


// Declaration

namespace ers::impl {
    template<typename T>
    using Fnv1AHash = THashBase<T, fnv_1a_hash_policy>;
}

namespace ers {
    using impl::fnv_1a_hash_policy;
    using impl::Fnv1AHash;
}
