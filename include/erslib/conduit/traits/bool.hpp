#pragma once

// std
#include <string>
#include <string_view>

// pqxx
#include <pqxx/params>
#include <pqxx/row>

// ers
#include <erslib/conduit/impl/generator.hpp>
#include <erslib/conduit/impl/traits.hpp>


template<>
struct conduit::impl::sql_type<bool> {
    static constexpr std::string_view name = "BOOLEAN";
    static constexpr bool nullable = false;
};

template<>
struct conduit::impl::sql_value<bool> {
    static std::string to_text(bool what) {
        return what ? "t" : "f";
    }

    static void bind(pqxx::params& out, bool what) {
        out.append(what);
    }

    static bool read(const auto& field) {
        return field.template as<bool>();
    }
};


template<>
struct conduit::impl::row_reader<bool> {
    struct state {};

    static state prepare(const pqxx::result&) { return {}; }

    static bool read(pqxx::row_ref row, const state&) {
        return sql_value<bool>::read(row[0]);
    }
};
