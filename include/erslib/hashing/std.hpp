#pragma once

// ers
#include <erslib/hashing/base.hpp>

// algos usage

namespace ers::hashing {
    struct std_policy {};
}

template<>
struct ers::internal::backend<ers::hashing::std_policy> {
    template<typename T>
    static constexpr size_t process_value(
        const T& value,
        size_t /*seed*/
    ) noexcept {
        return std::hash<T> {}(value);
    }
};

namespace ers {
    template<typename T>
    using Hash = THashBase<T, hashing::std_policy>;
}
