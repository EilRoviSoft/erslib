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
#include <erslib/flux/impl/generator.hpp>
#include <erslib/flux/impl/traits.hpp>


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
        auto r = ers::uuid_from_string(field.template as<std::string_view>());

        if (!r)
            throw ers::make_parse_error("{}", r.error().message());

        return *r;
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
