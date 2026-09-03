#pragma once

// ers
#include <erslib/core/type/optional.hpp>
#include <erslib/conduit/impl/generator.hpp>
#include <erslib/conduit/impl/traits.hpp>


template<typename T>
struct conduit::impl::sql_type<ers::optional<T>> : sql_type<T> {
    static constexpr bool nullable = true;
};

template<typename T>
struct conduit::impl::sql_value<ers::optional<T>> {
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

template<conduit::impl::ReadableRow T>
struct conduit::impl::sql_collector<ers::optional<T>> {
    using row_t = T;

    static ers::optional<T> collect(const pqxx::result& from) {
        return read_at_most_one<T>(from);
    }
};
