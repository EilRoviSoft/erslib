#pragma once

// ers
#include <erslib/core/type/optional.hpp>
#include <erslib/dbio/generator.hpp>
#include <erslib/dbio/impl/traits.hpp>


template<typename T>
struct dbio::impl::sql_type<ers::optional<T>> : sql_type<T> {
    static constexpr bool nullable = true;
};

template<typename T>
struct dbio::impl::sql_value<ers::optional<T>> {
    static void bind(pqxx::params& out, const ers::optional<T>& what) {
        if (what)
            sql_value<T>::bind(out, *what);
        else
            out.append();
    }

    static ers::optional<T> read(const auto& field) {
        if (field.is_null())
            return ers::nullopt;

        return sql_value<T>::read(field);
    }
};

template<dbio::ReadableRow T>
struct dbio::impl::sql_collector<ers::optional<T>> {
    using row_t = T;

    static ers::Result<ers::optional<T>> collect(const pqxx::result& from) {
        return read_at_most_one<T>(from);
    }
};
