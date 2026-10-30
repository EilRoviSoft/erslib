#pragma once

// std
#include <utility>
#include <vector>

// ers
#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/query/clause.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    class ERSLIB_EXPORT ValuesClause : public IClause {
    public:
        // Member functions

        explicit ValuesClause(std::vector<Expr> items);


        // Executors

        [[nodiscard]]
        ers::Status render(Context& ctx) const override;


    private:
        std::vector<Expr> _items;
    };


    namespace clauses {
        // A value is bound, an Expr is rendered: values(url, now()).
        template<typename... Args>
        Clause values(Args&&... args) {
            std::vector<Expr> items;
            items.reserve(sizeof...(Args));
            (items.push_back(Expr(std::forward<Args>(args))), ...);
            return make_clause<ValuesClause>(std::move(items));
        }
    }
}
