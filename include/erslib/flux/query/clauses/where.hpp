#pragma once

// std
#include <concepts>
#include <utility>

// ers
#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/query/clause.hpp>
#include <erslib/flux/query/slots/having.hpp>
#include <erslib/flux/query/slots/where.hpp>

// export
#include <erslib/export.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    // A condition joined with the others of its slot by AND: WHERE, HAVING.
    class ERSLIB_EXPORT ConditionClause : public IClause {
    public:
        // Member functions

        ConditionClause(SlotRef slot, Expr condition);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        Expr _condition;
    };


    namespace clauses {
        // Expr only: 'where("flag")' must not silently bind a string.
        template<std::same_as<Expr> T>
        Clause where(T condition) {
            return make_clause<ConditionClause>(&slots::where, std::move(condition));
        }

        template<std::same_as<Expr> T>
        Clause having(T condition) {
            return make_clause<ConditionClause>(&slots::having, std::move(condition));
        }
    }
}
