#pragma once

// ers
#include <erslib/dbio/generator.hpp>
#include <erslib/dbio/impl/traits.hpp>

// pqxx
#include <pqxx/row.hxx>


template<>
struct dbio::impl::sql_type<bool> {
    static constexpr std::string_view name = "BOOLEAN";
    static constexpr bool nullable = false;
};


template<>
struct dbio::impl::row_reader<bool> {
    struct state {};

    static state prepare(const pqxx::result&) { return {}; }
    static bool read(pqxx::row_ref row, const state&) { return row[0].as<bool>(); }
};
