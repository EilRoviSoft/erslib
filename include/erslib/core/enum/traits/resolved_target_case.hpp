#pragma once

// ers
#include <erslib/core/enum/traits/enumerator_style.hpp>


namespace ers::impl {
    template<typename E, E Value>
    struct resolved_target_case {
        using type = typename enumerator_style<E, Value>::target_case;
    };
}
