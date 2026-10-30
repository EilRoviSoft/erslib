#pragma once

// std
#include <string>
#include <utility>
#include <vector>

// ers
#include <erslib/flux/query/clauses/column.hpp>
#include <erslib/flux/query/clauses/values.hpp>
#include <erslib/flux/query/layouts/insert.hpp>
#include <erslib/flux/query/query_builder.hpp>
#include <erslib/flux/reflect/crud/detail.hpp>
#include <erslib/flux/reflect/schema.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::crud {
    // Everything but the identity and the read-only columns.
    template<reflect::Entity T>
    QueryBuilder insert(const T& what) {
        static_assert(reflect::metadata_is_valid<T>, "DeclarationMetadata names a column that does not exist on the entity.");
        static_assert(!reflect::insert_members<T>.empty(), "The entity has no column to insert.");

        std::vector<Clause> columns;
        std::vector<Expr> values;

        template for (constexpr auto m : reflect::insert_members<T>) {
            columns.push_back(clauses::column(std::string(reflect::column_name<m>)));
            values.push_back(Expr(what.[:m:]));
        }

        return layouts::insert_into(table_of<T>())
            | std::move(columns)
            | make_clause<ValuesClause>(std::move(values));
    }
}
