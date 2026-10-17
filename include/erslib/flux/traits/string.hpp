#pragma once

// std
#include <string>

// ers
#include <erslib/flux/sql/value.hpp>


template<>
struct flux::impl::sql_type<std::string> {
    static constexpr std::string_view name = "TEXT";
    static constexpr bool nullable = false;
};
