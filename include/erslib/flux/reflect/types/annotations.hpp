#pragma once

// std
#include <string_view>

// ers
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/flux/reflect/types/entry.hpp>


namespace flux::impl::reflect {
    template<ers::fixed_string Name = "", ers::fixed_string Type = "">
    struct column_t {
        using kind = kinds::EColumn;
        static constexpr std::string_view name = Name.to_sv();
        static constexpr std::string_view type = Type.to_sv();
    };

    template<ers::fixed_string Name = "", ers::fixed_string Type = "">
    inline constexpr column_t<Name, Type> Column {};


    struct skip_t {};

    inline constexpr skip_t Skip {};
}
