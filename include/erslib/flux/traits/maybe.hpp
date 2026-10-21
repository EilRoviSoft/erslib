#pragma once

// ers
#include <erslib/core/type/maybe.hpp>
#include <erslib/flux/result/collector.hpp>
#include <erslib/flux/result/row.hpp>
#include <erslib/flux/sql/value.hpp>


template<typename T>
struct flux::impl::sql_type<ers::maybe<T>> : sql_type<T> {
    static constexpr bool nullable = true;
};

template<typename T>
struct flux::impl::sql_value<ers::maybe<T>> {
    static void bind(pqxx::params& out, const ers::maybe<T>& what) {
        if (what)
            sql_value<T>::bind(out, *what);
        else
            out.append();
    }

    static ers::maybe<T> read(const auto& field) {
        if (field.is_null())
            return ers::none;

        return sql_value<T>::read(field);
    }
};

template<flux::impl::ReadableRow T>
struct flux::impl::sql_collector<ers::maybe<T>> {
    using row_t = T;

    static ers::maybe<T> collect(const pqxx::result& from) {
        return read_at_most_one<T>(from);
    }
};
