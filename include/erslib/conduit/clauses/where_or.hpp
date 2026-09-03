#pragma once

// std
#include <vector>

// ers
#include <erslib/conduit/clauses/where.hpp>


namespace conduit::impl {
    class ERSLIB_EXPORT WhereOrClause : public IWhereClause {
    public:
        // Member functions

        explicit WhereOrClause(std::vector<Clause> branches);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::vector<Clause> _branches;
    };


    namespace clauses {
        template<typename... Args>
        Clause where_or(Args&&... branches) {
            std::vector<Clause> v;
            v.reserve(sizeof...(Args));
            (v.emplace_back(std::forward<Args>(branches)), ...);
            return make_clause<WhereOrClause>(std::move(v));
        }
    }
}
