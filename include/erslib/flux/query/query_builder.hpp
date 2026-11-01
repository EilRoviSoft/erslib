#pragma once

// std
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/flux/query/clause.hpp>
#include <erslib/flux/query/layout.hpp>
#include <erslib/flux/result/query_result.hpp>
#include <erslib/flux/sql/context.hpp>

// export
#include <erslib/export.hpp>


// Implementation --------------------------------------------------------------

// Query

namespace flux::impl {
    struct WithCte;
    struct WithUnion;
}

namespace flux::impl {
    class ERSLIB_EXPORT QueryBuilder {
    public:
        // Member functions

        explicit QueryBuilder(LayoutView layout) :
            _layout(layout) {
        }


        // Accessors

        Layout& layout() noexcept { return _layout; }
        const Layout& layout() const noexcept { return _layout; }


        // Modifiers

        void add(Clause clause);
        void add(std::vector<Clause> clauses);

        void add(WithCte with_cte);
        void add(WithUnion with_union);


        ers::Status build(Context& ctx) const;

        ers::Result<std::string> to_sql() const;


        // Executors

        QueryResult exec(pqxx::dbtransaction& tx) const;


    private:
        Layout _layout;
        std::vector<Clause> _clauses;
        std::vector<WithCte> _ctes;
        std::vector<WithUnion> _unions;


        ers::Status _render_slot(Context& ctx, const SlotBinding& binding) const;
        ers::Status _render_custom(Context& ctx, const SlotBinding& binding) const;


        ers::Status _validate() const;

        bool _has(SlotRef slot) const;
    };
}


// Exclusive clauses

namespace flux::impl {
    struct WithCte {
        QueryBuilder definition;
        std::string name;
        bool recursive;
    };

    struct WithUnion {
        QueryBuilder other;
        bool all;
    };


    namespace clauses {
        ERSLIB_EXPORT WithCte with_cte(std::string name, QueryBuilder definition, bool recursive = false);

        ERSLIB_EXPORT WithUnion union_with(QueryBuilder other, bool all = false);
    }
}



// Operators

namespace flux::impl {
    QueryBuilder& operator|(QueryBuilder& lhs, Clause rhs);
    QueryBuilder&& operator|(QueryBuilder&& lhs, Clause rhs);
    QueryBuilder& operator|=(QueryBuilder& lhs, Clause rhs);

    QueryBuilder& operator|(QueryBuilder& lhs, std::vector<Clause> rhs);
    QueryBuilder&& operator|(QueryBuilder&& lhs, std::vector<Clause> rhs);
    QueryBuilder& operator|=(QueryBuilder& lhs, std::vector<Clause> rhs);

    QueryBuilder& operator|(QueryBuilder& lhs, WithUnion rhs);
    QueryBuilder&& operator|(QueryBuilder&& lhs, WithUnion rhs);
    QueryBuilder& operator|=(QueryBuilder& lhs, WithUnion rhs);

    QueryBuilder& operator|(QueryBuilder& lhs, WithCte rhs);
    QueryBuilder&& operator|(QueryBuilder&& lhs, WithCte rhs);
    QueryBuilder& operator|=(QueryBuilder& lhs, WithCte rhs);
}


// Subqueries

namespace flux::impl {
    ERSLIB_EXPORT ers::Status append_subquery(
        Context& ctx, const QueryBuilder& sub,
        std::string_view open, std::string_view close = ")"
    );
}
