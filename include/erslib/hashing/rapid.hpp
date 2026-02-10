#pragma once

// ers
#include <erslib/hashing/base.hpp>

// implementation

#include "rapid-impl.hpp"

// algos usage

namespace ers::hashing {
    struct rapid_policy {};
    struct rapid_micro_policy {};
    struct rapid_nano_policy {};
}

template<>
struct ers::internal::backend<ers::hashing::rapid_policy> {
    static constexpr size_t process(
        const void* data,
        size_t size,
        size_t seed
    ) noexcept {
        return rapidhash_withSeed(data, size, seed);
    }
};

template<>
struct ers::internal::backend<ers::hashing::rapid_micro_policy> {
    static constexpr size_t process(
        const void* data,
        size_t size,
        size_t seed
    ) noexcept {
        return rapidhashMicro_withSeed(data, size, seed);
    }
};

template<>
struct ers::internal::backend<ers::hashing::rapid_nano_policy> {
    static constexpr size_t process(
        const void* data,
        size_t size,
        size_t seed
    ) noexcept {
        return rapidhashNano_withSeed(data, size, seed);
    }
};

// declaration

namespace ers {
    template<typename T>
    using RapidHash = THashBase<T, hashing::rapid_policy>;

    template<typename T>
    using RapidMicroHash = THashBase<T, hashing::rapid_micro_policy>;

    template<typename T>
    using RapidNanoHash = THashBase<T, hashing::rapid_nano_policy>;
}
