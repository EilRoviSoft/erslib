#pragma once

// std
#include <string>
#include <utility>
#include <vector>

// ers
#include <erslib/dbio/clauses/conflict.hpp>
#include <erslib/dbio/layouts/insert.hpp>
#include <erslib/dbio/sql/columns.hpp>
#include <erslib/dbio/sql/statement.hpp>


namespace dbio::impl::sql {
    template<Entity T>
    struct statement<T, kind::Save> {
        static Query build(const T& what) {
            static_assert(!conflict_constraint<T>().empty(),
                "save() cannot pick a conflict target: this entity has an identity column and "
                "several Unique<> constraints. Add a Conflict<...> member naming the one to upsert on.");

            Query out = layouts::insert_into(std::string(table_name<T>()));
            out |= column_list<T, kind::Save, role::Write>();
            out |= values_of<T, kind::Save, role::Write>(what);
            out |= clauses::on_conflict_constraint(std::string(conflict_constraint<T>()));

            
            std::vector<std::string> assigned = column_names<T, kind::Save, role::Key>();

            if (assigned.empty())
                out |= clauses::do_nothing();
            else
                out |= clauses::do_update(std::move(assigned));

            out |= returning_list<T, kind::Save, role::Read>();

            return out;
        }
    };


    template<Entity T>
    Query save(const T& what) {
        return statement<T, kind::Save>::build(what);
    }
}
