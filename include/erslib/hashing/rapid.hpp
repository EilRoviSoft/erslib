#pragma once

// ers
#include <erslib/hashing/base.hpp>

// implementation

namespace ers::hashing::_impl {
#include "rapid-impl.hpp"
}

// algos usage

namespace ers::hashing::_impl {
    struct rapid_tag {};
    struct rapid_micro_tag {};
    struct rapid_nano_tag {};
}

template<>
struct ers::hashing::_impl::backend<ers::hashing::_impl::rapid_tag> {
    static constexpr size_t process(
        const void* data,
        size_t size,
        size_t seed
    ) noexcept {
        return rapidhash_withSeed(data, size, seed);
    }
};

template<>
struct ers::hashing::_impl::backend<ers::hashing::_impl::rapid_micro_tag> {
    static constexpr size_t process(
        const void* data,
        size_t size,
        size_t seed
    ) noexcept {
        return rapidhashMicro_withSeed(data, size, seed);
    }
};

template<>
struct ers::hashing::_impl::backend<ers::hashing::_impl::rapid_nano_tag> {
    static constexpr size_t process(
        const void* data,
        size_t size,
        size_t seed
    ) noexcept {
        return rapidhashNano_withSeed(data, size, seed);
    }
};

// declaration

namespace ers::hashing {
    template<typename T>
    using Rapid = HashBase<T, _impl::rapid_tag>;

    template<typename T>
    using RapidMicro = HashBase<T, _impl::rapid_micro_tag>;

    template<typename T>
    using RapidNano = HashBase<T, _impl::rapid_nano_tag>;
}

namespace ers {
    template<typename T>
    constexpr hashing::Rapid<T> rapidhash {};

    template<typename T>
    constexpr hashing::RapidMicro<T> rapidhash_micro {};

    template<typename T>
    constexpr hashing::RapidNano<T> rapidhash_nano {};
}
