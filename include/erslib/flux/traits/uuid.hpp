#pragma once

#ifdef _HAS_BOOST_UUID

// std
#include <string>
#include <string_view>

// pqxx
#include <pqxx/params>
#include <pqxx/row>

// ers
#include <erslib/core/exception.hpp>
#include <erslib/core/type/uuid.hpp>
#include <erslib/flux/result/row.hpp>
#include <erslib/flux/sql/value.hpp>


template<>
struct flux::impl::sql_type<ers::uuid> {
    static constexpr std::string_view name = "UUID";
    static constexpr bool nullable = false;
};

template<>
struct flux::impl::sql_value<ers::uuid> {
    static std::string to_text(const ers::uuid& what) {
        return ers::uuid_to_string(what);
    }

    static void bind(pqxx::params& out, const ers::uuid& what) {
        out.append(to_text(what));
    }

    static ers::uuid read(const auto& field) {
        const auto text = field.template as<std::string_view>();
        return ers::try_uuid_from_string(text).template or_throw<ers::parse_error>();
    }
};


template<>
struct flux::impl::row_reader<ers::uuid> {
    struct state {};

    static state prepare(const pqxx::result&) { return {}; }

    static ers::uuid read(pqxx::row_ref row, const state&) {
        return sql_value<ers::uuid>::read(row[0]);
    }
};

#endif
