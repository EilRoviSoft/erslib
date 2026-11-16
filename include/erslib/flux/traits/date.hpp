#pragma once

// std
#include <string>
#include <string_view>

// pqxx
#include <pqxx/row>

// ers
#include <erslib/core/convert/pg_time.hpp>
#include <erslib/core/exception.hpp>
#include <erslib/core/type/time.hpp>
#include <erslib/flux/result/row.hpp>
#include <erslib/flux/sql/value.hpp>


template<>
struct flux::impl::sql_type<ers::date_t> {
    static constexpr std::string_view name = "DATE";
    static constexpr bool nullable = false;
};

template<>
struct flux::impl::sql_value<ers::date_t> {
    static std::string to_text(const ers::date_t& what) {
        return ers::date_to_iso8601(what);
    }

    static void bind(pqxx::params& out, ers::date_t what) {
        out.append(ers::date_to_iso8601(what));
    }

    static ers::date_t read(const auto& field) {
        return ers::date_from_iso8601(field.template as<std::string_view>())
            .template or_throw<ers::parse_error>();
    }
};

template<>
struct flux::impl::row_reader<ers::date_t> {
    struct state {};

    static state prepare(const pqxx::result&) { return {}; }

    static ers::date_t read(pqxx::row_ref row, const state&) {
        return sql_value<ers::date_t>::read(row[0]);
    }
};
