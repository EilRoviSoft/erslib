#pragma once

// std
#include <string>
#include <utility>

// ers
#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/query/clause.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    class ERSLIB_EXPORT AssignClause : public IClause {
    public:
        // Member functions

        AssignClause(std::string column, Expr value);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _column;
        Expr _value;
    };


    namespace clauses {
        // A value is bound, an Expr is rendered: assign("stock", col("stock") - qty).
        template<typename T>
        Clause assign(std::string column, T&& value) {
            return make_clause<AssignClause>(std::move(column), Expr(std::forward<T>(value)));
        }

        ERSLIB_EXPORT Clause assign_null(std::string column);


        template<typename T>
        Clause set(std::string column, T&& value) {
            return assign(std::move(column), std::forward<T>(value));
        }
    }
}
