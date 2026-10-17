#pragma once

// std
#include <string>

// pqxx
#include <pqxx/row>

// ers
#include <erslib/core/convert/pg_time.hpp>
#include <erslib/core/exception.hpp>
#include <erslib/core/type/time.hpp>
#include <erslib/flux/impl/generator.hpp>
#include <erslib/flux/impl/traits.hpp>


template<>
struct flux::impl::sql_type<ers::timestamp_t> {
    static constexpr std::string_view name = "TIMESTAMPTZ";
    static constexpr bool nullable = false;
};

template<>
struct flux::impl::sql_value<ers::timestamp_t> {
    static std::string to_text(const ers::timestamp_t& what) {
        return ers::timestamp_to_iso8601(what);
    }

    static void bind(pqxx::params& out, ers::timestamp_t what) {
        out.append(ers::timestamp_to_iso8601(what));
    }

    static ers::timestamp_t read(const auto& field) {
        auto r = ers::timestamp_from_iso8601(field.template as<std::string_view>());

        if (!r)
            throw ers::make_parse_error("{}", r.error().message());

        return *r;
    }
};


template<>
struct flux::impl::row_reader<ers::timestamp_t> {
    struct state {};

    static state prepare(const pqxx::result&) { return {}; }

    static ers::timestamp_t read(pqxx::row_ref row, const state&) {
        return sql_value<ers::timestamp_t>::read(row[0]);
    }
};
