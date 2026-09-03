#pragma once

// std
#include <string>

// ers
#include <erslib/conduit/clauses/where.hpp>
#include <erslib/conduit/impl/op.hpp>
#include <erslib/conduit/impl/traits.hpp>


namespace conduit::impl {
    class ERSLIB_EXPORT WhereOpClause : public IWhereClause {
    public:
        // Member functions

        WhereOpClause(std::string column, EOp op, binder_t binder);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _column;
        EOp _op;
        binder_t _binder;
    };


    namespace clauses {
        template<typename T>
        Clause where(std::string column, EOp op, T&& value) {
            return make_clause<WhereOpClause>(std::move(column), op, make_binder(std::forward<T>(value)));
        }
    }
}
