#pragma once

// std
#include <cstddef>

// ers
#include <erslib/core/type/general.hpp>


namespace ers::impl::hashing {
    inline constexpr u64 fnv_offset_basis = 0xcbf29ce484222325ull;

    constexpr u64 fnv_step(u64 hash, u64 byte) {
        hash ^= byte;
        hash *= 0x100000001b3ull;
        return hash;
    }

    constexpr u64 fnv_1a(const std::byte* p, size_t len, size_t seed) {
        u64 hash = fnv_offset_basis;

        if (seed) {
            for (int i = 0; i < 8; ++i)
                hash = fnv_step(hash, (seed >> (8 * i)) & 0xff);
        }
        
        for (size_t i = 0; i < len; ++i)
            hash = fnv_step(hash, std::to_integer<u64>(p[i]));

        return hash;
    }
}
