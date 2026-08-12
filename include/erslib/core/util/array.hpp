#pragma once

// std
#include <array>


namespace ers::impl::util {
    template<typename T, size_t... Is>
    constexpr auto elems_as_array(T v, std::index_sequence<Is...>) {
        return std::array { v[Is]... };
    }
}


// Exports

namespace ers {
    namespace util = impl::util;
}
