#pragma once

// std
#include <concepts>
#include <utility>

// ers
#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/query/clauses/where.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    class ERSLIB_EXPORT WhereExprClause : public IWhereClause {
    public:
        // Member functions

        explicit WhereExprClause(Expr condition);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        Expr _condition;
    };


    namespace clauses {
        // Expr only: 'where("flag")' must not silently bind a string
        template<std::same_as<Expr> T>
        Clause where(T condition) {
            return make_clause<WhereExprClause>(std::move(condition));
        }
    }
}
