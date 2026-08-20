#pragma once

// std
#include <string>
#include <utility>

// ers
#include <erslib/dbio/clauses/bound.hpp>
#include <erslib/dbio/layouts/select.hpp>
#include <erslib/dbio/sql/columns.hpp>
#include <erslib/dbio/sql/statement.hpp>


namespace dbio::impl::sql {
    template<Entity T>
    struct statement<T, kind::Select> {
        static Query build() {
            Query out = layouts::select_from(std::string(table_name<T>()));
            out |= column_list<T, kind::Select, role::Read>();
            return out;
        }
    };

    template<Entity T>
    struct statement<T, kind::LoadByPk> {
        template<typename... Keys>
        static Query build(Keys&&... keys) {
            Query out = layouts::select_from(std::string(table_name<T>()));
            out |= column_list<T, kind::LoadByPk, role::Read>();
            out |= conditions_for<T>(std::forward<Keys>(keys)...);
            out |= clauses::with_limit(1);
            return out;
        }
    };


    template<Entity T>
    Query select_all() {
        return statement<T, kind::Select>::build();
    }

    template<Entity T, typename... Keys>
    Query load_by_pk(Keys&&... keys) {
        return statement<T, kind::LoadByPk>::build(std::forward<Keys>(keys)...);
    }
}
