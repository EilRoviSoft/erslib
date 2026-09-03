#pragma once

// ers
#include <erslib/conduit/clauses/where.hpp>


namespace conduit::impl {
    class ERSLIB_EXPORT WhereBetweenClause : public IWhereClause {
    public:
        WhereBetweenClause(std::string column, binder_t low, binder_t high);

        ers::Status render(Context& ctx) const override;


    private:
        std::string _column;
        binder_t _low, _high;
    };

    namespace clauses {
        template<typename A, typename B>
        Clause where_between(std::string column, A&& low, B&& high) {
            return make_clause<WhereBetweenClause>(
                std::move(column),
                make_binder(std::forward<A>(low)),
                make_binder(std::forward<B>(high))
            );
        }
    }
}
