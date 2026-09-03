#pragma once

// std
#include <string>

// ers
#include <erslib/conduit/impl/traits.hpp>


template<>
struct conduit::impl::sql_type<std::string> {
    static constexpr std::string_view name = "TEXT";
    static constexpr bool nullable = false;
};
