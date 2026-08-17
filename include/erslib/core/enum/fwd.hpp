#pragma once

// ers
#include <erslib/core/type/fixed_string.hpp>


namespace ers::impl::enum_utils {
    struct as_is_t {};
    inline constexpr as_is_t as_is {};

    template<fixed_string Str>
    struct custom_t {
        static constexpr auto value = Str;
    };

    template<fixed_string Str>
    inline constexpr custom_t<Str> custom {};


    template<typename Style>
    struct source_case_t {};

    template<typename Style>
    inline constexpr source_case_t<Style> source_case {};

    template<typename Style>
    struct target_case_t {};

    template<typename Style>
    inline constexpr target_case_t<Style> target_case {};

    template<typename Source, typename Target>
    struct set_cases_t {};

    template<typename Source, typename Target>
    inline constexpr set_cases_t<Source, Target> set_cases {};
}


namespace ers::impl {
    struct pascal_case_t;
}
