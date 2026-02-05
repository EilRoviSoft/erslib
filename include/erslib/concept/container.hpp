#pragma once

// std
#include <type_traits>

namespace ers {
    template<typename T>
    concept ContiguousContainer = requires(const T t) {
        { t.data() } -> std::is_pointer; // Must have .data() returning a pointer
        { t.size() } -> size_t;          // Must have .size() returning an unsigned int
    };
}
