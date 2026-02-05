#pragma once

// ers
#include <erslib/hashing/base.hpp>

// algos usage

namespace ers::hashing::_impl {
    struct std_tag {};
}

template<>
struct ers::hashing::_impl::backend<ers::hashing::_impl::std_tag> {
    template<typename T>
    static constexpr size_t process_value(
        const T& value,
        size_t /*seed*/
    ) noexcept {
        return std::hash<T> {}(value);
    }
};

namespace ers::hashing {
    template<typename T>
    using Std = HashBase<T, _impl::std_tag>;
}

namespace ers {
    template<typename T>
    constexpr hashing::Std<T> stdhash;
}
