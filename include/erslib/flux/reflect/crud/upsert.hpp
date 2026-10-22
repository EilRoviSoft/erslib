#pragma once

// std
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/flux/query/clauses/conflict.hpp>
#include <erslib/flux/query/query_builder.hpp>
#include <erslib/flux/reflect/crud/detail.hpp>
#include <erslib/flux/reflect/crud/insert.hpp>
#include <erslib/flux/reflect/schema.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::crud {
    // On the conflict with the primary key, or with a unique constraint of the declaration:
    // upsert<"images_url_key">(img). An entity with an identity key needs the constraint.
    template<ers::fixed_string Constraint = "", reflect::Entity T>
    QueryBuilder upsert(const T& what) {
        QueryBuilder out = insert(what);

        if constexpr (Constraint.to_sv().empty()) {
            static_assert(!reflect::pk_members<T>.empty(), "The entity has no primary key.");
            static_assert(!reflect::has_identity<T>, "The key is generated, name a unique constraint: upsert<\"name\">(what).");

            std::vector<std::string> keys;

            for (const std::string_view name : reflect::primary_key<T>)
                keys.emplace_back(name);

            out |= clauses::on_conflict(keys);
        } else {
            static_assert(reflect::has_unique_constraint<T>(Constraint.to_sv()), "The declaration has no such PRIMARY KEY or UNIQUE constraint.");

            out |= clauses::on_conflict_constraint(std::string(Constraint.to_sv()));
        }

        std::vector<std::string> updated;

        template for (constexpr auto m : reflect::update_members<T>)
            updated.emplace_back(reflect::column_name<m>);

        // Nothing to update: touch a column anyway, so RETURNING still gives the row.
        if (updated.empty())
            updated.emplace_back(reflect::column_name<reflect::insert_members<T>.front()>);

        out |= clauses::do_update(updated);
        return out;
    }
}
