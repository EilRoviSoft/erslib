#pragma once

// std
#include <string>
#include <string_view>

// pqxx
#include <pqxx/params>
#include <pqxx/row>

// ers
#include <erslib/flux/result/row.hpp>
#include <erslib/flux/sql/value.hpp>


template<>
struct flux::impl::sql_type<bool> {
    static constexpr std::string_view name = "BOOLEAN";
    static constexpr bool nullable = false;
};

template<>
struct flux::impl::sql_value<bool> {
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
struct flux::impl::row_reader<bool> {
    struct state {};

    static state prepare(const pqxx::result&) { return {}; }

    static bool read(pqxx::row_ref row, const state&) {
        return sql_value<bool>::read(row[0]);
    }
};
