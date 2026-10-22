#pragma once

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/query/clause.hpp>

// export
#include <erslib/export.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    enum class EOrder : u8 { Asc, Desc };


    class ERSLIB_EXPORT OrderClause : public IClause {
    public:
        // Member functions

        OrderClause(Expr what, EOrder order);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        Expr _what;
        EOrder _order;
    };


    namespace clauses {
        ERSLIB_EXPORT Clause order_by(ColumnExpr what, EOrder order = EOrder::Asc);
        ERSLIB_EXPORT Clause order_by_random();

        // The same order for the same seed, for paging through a shuffled list.
        ERSLIB_EXPORT Clause order_by_stable_random(ColumnExpr key, i64 seed);
    }
}
