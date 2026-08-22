#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/impl/traits.hpp>


template<>
struct dbio::impl::sql_type<std::string> {
    static constexpr std::string_view name = "TEXT";
    static constexpr bool nullable = false;
};
