#pragma once

// std
#include <ranges>
#include <vector>

// ers
#include <erslib/conduit/clauses/where.hpp>
#include <erslib/conduit/impl/binder.hpp>


namespace conduit::impl {
    class ERSLIB_EXPORT WhereInClause : public IWhereClause {
    public:
        // Member functions

        WhereInClause(std::string column, std::vector<binder_t> binders, bool negated);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _column;
        std::vector<binder_t> _binders;
        bool _negated;
    };


    namespace clauses {
        template<std::ranges::input_range R>
        Clause where_in(std::string column, R&& values, bool negated = false) {
            std::vector<binder_t> binders;

            if constexpr (std::ranges::sized_range<R>)
                binders.reserve(std::ranges::size(values));

            for (auto&& value : values)
                binders.emplace_back(make_binder(std::forward<decltype(value)>(value)));

            return make_clause<WhereInClause>(std::move(column), std::move(binders), negated);
        }

        template<typename T>
        Clause where_in(std::string column, std::initializer_list<T> values, bool negated = false) {
            return where_in<std::initializer_list<T>>(std::move(column), std::move(values), negated);
        }
    }
}
