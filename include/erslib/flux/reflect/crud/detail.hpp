#pragma once

// std
#include <concepts>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

// ers
#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/query/clause.hpp>
#include <erslib/flux/query/clauses/where.hpp>
#include <erslib/flux/query/query_builder.hpp>
#include <erslib/flux/reflect/schema.hpp>


// Details ---------------------------------------------------------------------

namespace flux::impl::crud {
    // A mistake of the call, reported when the query is rendered.
    class FailClause final : public IClause {
    public:
        // Member functions

        FailClause(SlotRef slot, std::string message) :
            IClause(slot),
            _message(std::move(message)) {
        }


        // Executors

        ers::Status render(Context&) const override {
            return ers::make_error("{}", _message);
        }


    private:
        std::string _message;
    };


    template<reflect::Entity T>
    std::string table_of() {
        return std::string(reflect::qualified_name<T>);
    }

    // 'scope' of the metadata: static void scope(QueryBuilder&), adds the conditions that hold for every query.
    template<typename T>
    concept HasScope = requires(QueryBuilder& q) { reflect::DeclarationMetadata<T>::scope(q); };

    template<reflect::Entity T>
    void apply_scope(QueryBuilder& q) {
        if constexpr (HasScope<T>)
            reflect::DeclarationMetadata<T>::scope(q);
    }

    template<reflect::Entity T>
    Expr soft_delete_expr() {
        static_assert(reflect::has_soft_delete<T>, "The entity has no SoftDelete.");
        static_assert(reflect::soft_delete_is_nullable<T>(), "The SoftDelete column has to be nullable: NULL means the row lives.");

        return clauses::col(std::string(reflect::soft_delete_column<T>));
    }

    template<reflect::Entity T>
    void hide_deleted(QueryBuilder& q) {
        if constexpr (reflect::has_soft_delete<T>)
            q |= clauses::where(soft_delete_expr<T>().is_null());
    }

    template<reflect::Entity T>
    QueryBuilder scoped(QueryBuilder&& q) {
        apply_scope<T>(q);
        hide_deleted<T>(q);

        return std::move(q);
    }

    inline Expr all_of(std::vector<Expr> parts) {
        Expr out = std::move(parts.front());

        for (std::size_t i = 1; i < parts.size(); i++)
            out = out && std::move(parts[i]);

        return out;
    }

    template<typename T, typename... Keys>
    consteval bool keys_match() {
        return []<std::size_t... I>(std::index_sequence<I...>) {
            return (std::convertible_to<Keys, reflect::member_type<reflect::pk_members<T>[I]>> && ...);
        }(std::index_sequence_for<Keys...> {});
    }
}
