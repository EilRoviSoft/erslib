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

    struct pascal_case_t;
}
