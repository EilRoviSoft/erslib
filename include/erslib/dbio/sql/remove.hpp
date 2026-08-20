#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/layouts/delete.hpp>
#include <erslib/dbio/sql/columns.hpp>
#include <erslib/dbio/sql/statement.hpp>


namespace dbio::impl::sql {
    template<Entity T>
    struct statement<T, kind::Delete> {
        static Query build(const T& what) {
            Query out = layouts::delete_from(std::string(table_name<T>()));
            out |= conditions_of<T, kind::Delete, role::Key>(what);
            return out;
        }
    };


    template<Entity T>
    Query remove(const T& what) {
        return statement<T, kind::Delete>::build(what);
    }
}
