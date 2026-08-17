#pragma once

// ers
#include <erslib/core/meta.hpp>
#include <erslib/core/enum/case_styles.hpp>
#include <erslib/core/enum/fwd.hpp>


namespace ers::impl {
    template<typename E>
    consteval std::meta::info resolve_source_case_info() {
        if constexpr (meta::has_type_attribute2<^^E, enum_utils::set_cases_t>())
            return meta::get_type_attribute_arg2<^^E, enum_utils::set_cases_t, 0>();
        else if constexpr (meta::has_type_attribute<^^E, enum_utils::source_case_t>())
            return meta::get_type_attribute_arg<^^E, enum_utils::source_case_t>();
        else
            return ^^pascal_case_t;
    }

    template<typename E>
    consteval std::meta::info resolve_target_case_info() {
        if constexpr (meta::has_type_attribute2<^^E, enum_utils::set_cases_t>())
            return meta::get_type_attribute_arg2<^^E, enum_utils::set_cases_t, 1>();
        else if constexpr (meta::has_type_attribute<^^E, enum_utils::target_case_t>())
            return meta::get_type_attribute_arg<^^E, enum_utils::target_case_t>();
        else
            return ^^pascal_case_t;
    }


    template<typename E, E V>
    struct enumerator_style {
        using source_case = typename [: resolve_source_case_info<E>() :];
        using target_case = typename [: resolve_target_case_info<E>() :];

        static constexpr bool as_is = meta::has_attribute<meta::enumerator_of<E, V>(), enum_utils::as_is_t>();
        static constexpr bool custom = meta::has_template_attribute<meta::enumerator_of<E, V>(), enum_utils::custom_t>();
    };
}
