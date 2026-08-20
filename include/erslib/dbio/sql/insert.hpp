#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/layouts/insert.hpp>
#include <erslib/dbio/sql/columns.hpp>
#include <erslib/dbio/sql/statement.hpp>


namespace dbio::impl::sql {
    template<Entity T>
    struct statement<T, kind::Insert> {
        static Query build(const T& what) {
            Query out = layouts::insert_into(std::string(table_name<T>()));
            out |= column_list<T, kind::Insert, role::Write>();
            out |= values_of<T, kind::Insert, role::Write>(what);
            out |= returning_list<T, kind::Insert, role::Read>();
            return out;
        }
    };


    template<Entity T>
    Query insert(const T& what) {
        return statement<T, kind::Insert>::build(what);
    }
}
