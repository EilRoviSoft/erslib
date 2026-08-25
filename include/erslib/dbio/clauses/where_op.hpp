#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/clauses/where.hpp>
#include <erslib/dbio/impl/op.hpp>
#include <erslib/dbio/impl/traits.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT WhereOpClause : public IWhereClause {
    public:
        // Member functions

        WhereOpClause(std::string column, Op op, binder_t binder);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _column;
        Op _op;
        binder_t _binder;
    };


    namespace clauses {
        template<typename T>
        Clause where(std::string column, Op op, T&& value) {
            return make_clause<WhereOpClause>(std::move(column), op, make_binder(std::forward<T>(value)));
        }
    }
}
