#pragma once

// std
#include <concepts>
#include <utility>

// ers
#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/query/clause.hpp>

// export
#include <erslib/export.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    class ERSLIB_EXPORT WhereClause : public IClause {
    public:
        // Member functions

        explicit WhereClause(Expr condition);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        Expr _condition;
    };


    namespace clauses {
        // Expr only: 'where("flag")' must not silently bind a string.
        template<std::same_as<Expr> T>
        Clause where(T condition) {
            return make_clause<WhereClause>(std::move(condition));
        }
    }
}
