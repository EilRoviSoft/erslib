#pragma once

// std
#include <string>
#include <utility>
#include <vector>

// ers
#include <erslib/flux/query/clauses/column.hpp>
#include <erslib/flux/query/layouts/select.hpp>
#include <erslib/flux/query/query_builder.hpp>
#include <erslib/flux/reflect/crud/detail.hpp>
#include <erslib/flux/reflect/schema.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::crud {
    // The same as select() with the soft-deleted rows.
    template<reflect::Entity T, typename As = T>
    QueryBuilder select_all() {
        std::vector<Clause> columns;

        template for (constexpr auto m : reflect::column_list<As>) {
            static_assert(reflect::has_column<T>(reflect::column_name<m>), "A column of the projection isn't a column of the entity.");
            columns.push_back(clauses::column(std::string(reflect::column_name<m>)));
        }

        QueryBuilder out = layouts::select_from(table_of<T>()) | std::move(columns);
        apply_scope<T>(out);

        return out;
    }

    // An explicit column list, so a new column in the table never changes the result.
    // 'As' is a projection: a struct with some of the columns of T.
    // The scope of the metadata applies, the rows hidden by SoftDelete are skipped.
    template<reflect::Entity T, typename As = T>
    QueryBuilder select() {
        QueryBuilder out = select_all<T, As>();
        hide_deleted<T>(out);

        return out;
    }
}
