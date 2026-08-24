#pragma once

// std
#include <concepts>
#include <string_view>
#include <type_traits>

// ers
#include <erslib/dbio/impl/generator.hpp>
#include <erslib/dbio/impl/traits.hpp>


namespace dbio::impl {
    template<typename T>
    concept ScalarRow = (std::is_scalar_v<T> || std::convertible_to<T, std::string_view>)
        && requires(const pqxx::field& f) { { sql_value<T>::read(f) } -> std::same_as<T>; };
}

template<dbio::impl::ScalarRow T>
struct dbio::impl::row_reader<T> {
    struct state {};

    static state prepare(const pqxx::result&) { return {}; }
    static T read(pqxx::row_ref row, const state&) { return sql_value<T>::read(row[0]); }
};
