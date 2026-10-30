#pragma once

// std
#include <algorithm>
#include <concepts>
#include <meta>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// ers
#include <erslib/flux/query/clauses/assign.hpp>
#include <erslib/flux/query/clauses/where.hpp>
#include <erslib/flux/query/layouts/update.hpp>
#include <erslib/flux/query/query_builder.hpp>
#include <erslib/flux/query/slots/set.hpp>
#include <erslib/flux/reflect/crud/columns.hpp>
#include <erslib/flux/reflect/crud/condition.hpp>
#include <erslib/flux/reflect/crud/detail.hpp>
#include <erslib/flux/reflect/schema.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::crud {
    // Everything but the key, the identity and the read-only columns, by the key of the object.
    // The scope of the metadata applies, the rows hidden by SoftDelete are skipped.
    template<reflect::Entity T>
    QueryBuilder update(const T& what) {
        static_assert(reflect::metadata_is_valid<T>, "DeclarationMetadata names a column that does not exist on the entity.");
        static_assert(!reflect::update_members<T>.empty(), "The entity has no column to update.");

        std::vector<Clause> sets;

        template for (constexpr auto m : reflect::update_members<T>)
            sets.push_back(clauses::assign(std::string(reflect::column_name<m>), Expr(what.[:m:])));

        return scoped<T>(layouts::update(table_of<T>()) | std::move(sets) | clauses::where(where_pk(what)));
    }
}


// Partial update

namespace flux::impl::crud {
    // Typed, checked while compiling: update(img, only(cols.path, cols.url)).
    template<std::meta::info... Ms>
    struct OnlyColumns {};

    // By name, checked when the query is rendered: update(img, only("path", "url")).
    struct OnlyNames {
        std::vector<std::string> names;
    };


    template<std::meta::info... Ms>
        requires (sizeof...(Ms) > 0)
    constexpr OnlyColumns<Ms...> only(TypedColumn<Ms>...) {
        return {};
    }

    template<typename... Names>
        requires (sizeof...(Names) > 0 && (std::convertible_to<Names, std::string_view> && ...))
    OnlyNames only(Names&&... names) {
        return { { std::string(std::string_view(names))... } };
    }


    template<reflect::Entity T, std::meta::info... Ms>
    QueryBuilder update(const T& what, OnlyColumns<Ms...>) {
        static_assert(
            (reflect::is_update_member<T, Ms> && ...),
            "A column of 'only' can't be updated: it is a key, an identity, read-only, or not a column of the entity."
        );

        std::vector<Clause> sets;
        template for (constexpr auto m : { Ms... })
            sets.push_back(clauses::assign(std::string(reflect::column_name<m>), Expr(what.[:m:])));

        return scoped<T>(layouts::update(table_of<T>()) | std::move(sets) | clauses::where(where_pk(what)));
    }

    template<reflect::Entity T>
    QueryBuilder update(const T& what, OnlyNames only_names) {
        std::vector<Clause> sets;

        template for (constexpr auto m : reflect::update_members<T>) {
            if (const auto it = std::ranges::find(only_names.names, reflect::column_name<m>); it != only_names.names.end()) {
                sets.push_back(clauses::assign(std::string(reflect::column_name<m>), Expr(what.[:m:])));
                only_names.names.erase(it);
            }
        }

        auto out = scoped<T>(layouts::update(table_of<T>()) | std::move(sets) | clauses::where(where_pk(what)));

        if (!only_names.names.empty()) {
            std::string message = "Columns can't be updated, they are keys, identities, read-only or don't exist:";

            for (const auto& it : only_names.names)
                message += ' ' + it;

            out |= make_clause<FailClause>(&slots::set, std::move(message));
        }

        return out;
    }
}
