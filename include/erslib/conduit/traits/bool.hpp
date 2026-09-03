#pragma once

// ers
#include <erslib/conduit/impl/generator.hpp>
#include <erslib/conduit/impl/traits.hpp>

// pqxx
#include <pqxx/row.hxx>


template<>
struct conduit::impl::sql_type<bool> {
    static constexpr std::string_view name = "BOOLEAN";
    static constexpr bool nullable = false;
};


template<>
struct conduit::impl::row_reader<bool> {
    struct state {};

    static state prepare(const pqxx::result&) { return {}; }
    static bool read(pqxx::row_ref row, const state&) { return row[0].as<bool>(); }
};
