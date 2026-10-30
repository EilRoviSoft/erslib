#pragma once

// std
#include <cstddef>
#include <utility>
#include <vector>

// ers
#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/reflect/crud/columns.hpp>
#include <erslib/flux/reflect/crud/detail.hpp>
#include <erslib/flux/reflect/schema.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::crud {
    // 'id = $1' or 'product_id = $1 AND image_id = $2', the keys in the order of the primary key.
    template<reflect::Entity T, typename... Keys>
        requires (sizeof...(Keys) == reflect::pk_members<T>.size())
    Expr where_pk(Keys&&... keys) {
        static_assert(sizeof...(Keys) > 0, "The entity has no primary key.");
        static_assert(keys_match<T, Keys...>(), "A key doesn't convert to the type of its primary key field.");

        std::vector<Expr> parts;
        parts.reserve(sizeof...(Keys));

        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (parts.push_back(TypedColumn<reflect::pk_members<T>[I]> {} == std::forward<Keys>(keys)), ...);
        }(std::index_sequence_for<Keys...> {});

        return all_of(std::move(parts));
    }

    // The same with the keys taken from the object.
    template<reflect::Entity T>
    Expr where_pk(const T& what) {
        static_assert(!reflect::pk_members<T>.empty(), "The entity has no primary key.");

        std::vector<Expr> parts;
        parts.reserve(reflect::pk_members<T>.size());

        template for (constexpr auto m : reflect::pk_members<T>)
            parts.push_back(TypedColumn<m> {} == what.[:m:]);

        return all_of(std::move(parts));
    }
}
