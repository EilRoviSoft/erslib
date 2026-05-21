#pragma once

// ers
#include <erslib/type/fixed_string.hpp>


namespace ers::util {
    template<typename T, size_t... Is>
    constexpr auto elems_as_string_literal(T v, std::index_sequence<Is...>) {
        return fixed_string<sizeof...(Is) + 1>({ v[Is]..., '\0' });
    }
}
