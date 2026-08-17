#pragma once

#ifndef ERSLIB_HAS_REFLECTION
#  error "erslib/core/enum/traits/enumerator_style requires C++26 static reflection (configure with ERSLIB_ENABLE_REFLECTION=ON)"
#endif

// ers
#include <erslib/core/meta.hpp>
#include <erslib/core/enum/case_styles.hpp>
#include <erslib/core/enum/fwd.hpp>

namespace ers::impl {
    template<typename E>
    struct enum_traits {
        using source_case = enum_utils::pascal_case_t;
        using target_case = source_case;
    };


    template<typename E, E V>
    struct enumerator_style {
        using source_case = typename enum_traits<E>::source_case;
        using target_case = typename enum_traits<E>::target_case;

        static constexpr bool as_is = meta::has_attribute<enum_utils::as_is_t, E, V>();
        static constexpr bool custom = meta::has_template_attribute<enum_utils::custom_t, E, V>();
    };
}
