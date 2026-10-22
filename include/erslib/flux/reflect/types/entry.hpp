#pragma once

// std
#include <array>
#include <concepts>
#include <string>
#include <string_view>


namespace flux::impl::reflect::kinds {
    struct ESchema {};
    struct ETable {};
    struct EQuery {};

    struct EPk {};
    struct EUnique {};
    struct EFk {};
    struct ECheck {};

    struct EDefault {};
    struct EIdentity {};
    struct EReadOnly {};
    struct ESoftDelete {};
    struct EColumn {};
}

namespace flux::impl::reflect {
    template<typename T, typename Kind>
    concept EntryOf = requires { typename T::kind; }
        && std::same_as<typename T::kind, Kind>;
}


namespace flux::impl::reflect {
    struct constraint_context {
        std::string_view name;
        std::string_view schema;
    };

    struct entry_defaults {
        static constexpr std::array<std::string_view, 0> columns {};
        static constexpr std::array<std::string_view, 0> references {};
    };

    template<typename E>
    struct entry_traits : entry_defaults {};


    template<typename E>
    concept TableConstraint = requires(const constraint_context& ctx) {
        { entry_traits<E>::ddl(ctx) } -> std::convertible_to<std::string>;
    };

    template<typename E>
    concept ColumnModifier = requires {
        { entry_traits<E>::field } -> std::convertible_to<std::string_view>;
        { entry_traits<E>::column_ddl() } -> std::convertible_to<std::string>;
    };
}
