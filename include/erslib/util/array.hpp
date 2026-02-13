#pragma once

// std
#include <array>


namespace ers::util {
    template<typename T, std::size_t... Indexes>
    constexpr auto elems_as_array(T v, std::index_sequence<Indexes...>) {
        return std::array { v[Indexes]... };
    }
}
