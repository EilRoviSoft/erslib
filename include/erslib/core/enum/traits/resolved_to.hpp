#pragma once

// ers
#include <erslib/core/enum/traits/enumerator_style.hpp>


namespace ers::impl {
    template<typename E, E V>
    struct resolved_to {
        using type = typename enumerator_style<E, V>::to;
    };
}
