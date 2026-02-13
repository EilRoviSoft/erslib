#pragma once

// cpp
#include <type_traits>

// ers
#include <erslib/type/general.hpp>


namespace ers::internal {
    template<typename T, size_t Size, size_t Align>
    constexpr bool check_sbo() {
        return Size != 0
            && alignof(T) <= Align
            && sizeof(T) <= Size
            && std::is_nothrow_move_constructible_v<T>;
    }
}

namespace ers {
    template<typename T, size_t Size, size_t Align = alignof(T)>
    struct is_sbo_applicable : std::bool_constant<internal::check_sbo<T, Size, Align>()> {};

    template<typename T, size_t Size, size_t Align = alignof(T)>
    constexpr bool is_sbo_applicable_v = is_sbo_applicable<T, Size, Align>::value;

    enum class SboPolicy : u8 {
        Empty = 0,
        Dynamic,
        Embedded
    };
}
