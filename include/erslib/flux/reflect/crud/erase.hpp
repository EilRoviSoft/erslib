#pragma once

// std
#include <string>
#include <utility>

// ers
#include <erslib/flux/expr/functions.hpp>
#include <erslib/flux/query/clauses/assign.hpp>
#include <erslib/flux/query/clauses/where.hpp>
#include <erslib/flux/query/layouts/delete.hpp>
#include <erslib/flux/query/layouts/update.hpp>
#include <erslib/flux/query/query_builder.hpp>
#include <erslib/flux/reflect/crud/condition.hpp>
#include <erslib/flux/reflect/crud/detail.hpp>
#include <erslib/flux/reflect/schema.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::crud {
    // A real DELETE, with SoftDelete too.
    template<reflect::Entity T, typename... Keys>
        requires (sizeof...(Keys) == reflect::pk_members<T>.size())
    QueryBuilder erase_forever(Keys&&... keys) {
        QueryBuilder out = layouts::delete_from(table_of<T>()) | clauses::where(where_pk<T>(std::forward<Keys>(keys)...));
        apply_scope<T>(out);

        return out;
    }

    // Takes the mark of SoftDelete off.
    template<reflect::Entity T, typename... Keys>
        requires (sizeof...(Keys) == reflect::pk_members<T>.size())
    QueryBuilder restore(Keys&&... keys) {
        QueryBuilder out = layouts::update(table_of<T>())
            | clauses::assign_null(std::string(reflect::soft_delete_column<T>))
            | clauses::where(where_pk<T>(std::forward<Keys>(keys)...))
            | clauses::where(soft_delete_expr<T>().is_not_null());

        apply_scope<T>(out);
        return out;
    }

    // By the keys, in the order of the primary key:
    // with SoftDelete it only marks the row, the scope of the metadata applies.
    template<reflect::Entity T, typename... Keys>
        requires (sizeof...(Keys) == reflect::pk_members<T>.size())
    QueryBuilder erase(Keys&&... keys) {
        if constexpr (reflect::has_soft_delete<T>) {
            return scoped<T>(layouts::update(table_of<T>())
                | clauses::assign(std::string(reflect::soft_delete_column<T>), clauses::now())
                | clauses::where(where_pk<T>(std::forward<Keys>(keys)...)));
        } else
            return erase_forever<T>(std::forward<Keys>(keys)...);
    }
}
