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
        using from = pascal_case_t;
        using to = from;
    };


    template<typename E, E V>
    struct enumerator_style {
        using from = typename enum_traits<E>::from;
        using to = typename enum_traits<E>::to;

        static constexpr bool as_is = meta::has_attribute<meta::enumerator_of<E, V>(), enum_utils::as_is_t>();
        static constexpr bool custom = meta::has_template_attribute<meta::enumerator_of<E, V>(), enum_utils::custom_t>();
    };
}
