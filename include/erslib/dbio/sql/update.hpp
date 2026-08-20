#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/layouts/update.hpp>
#include <erslib/dbio/sql/columns.hpp>
#include <erslib/dbio/sql/statement.hpp>


namespace dbio::impl::sql {
    template<Entity T>
    struct statement<T, kind::Update> {
        static Query build(const T& what) {
            Query out = layouts::update(std::string(table_name<T>()));
            out |= assignments_of<T, kind::Update, role::Write>(what);
            out |= conditions_of<T, kind::Update, role::Key>(what);
            return out;
        }
    };


    template<Entity T>
    Query update(const T& what) {
        return statement<T, kind::Update>::build(what);
    }
}
