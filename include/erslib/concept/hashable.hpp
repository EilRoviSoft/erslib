#pragma once

// ers
#include <erslib/hashing/base.hpp>


namespace ers {
    template<typename T, typename HashTag>
    concept Hashable = requires(const T& value, size_t seed) {
        requires !std::is_same_v<
            decltype(internal::backend<HashTag>::process_value(value, seed)),
            size_t
        >;

        { internal::backend<HashTag>::process_value(value, seed) } -> std::same_as<size_t>;

        requires noexcept(internal::backend<HashTag>::process_value(value, seed));
    };
}
