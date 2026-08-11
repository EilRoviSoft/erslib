#pragma once

// std
#include <initializer_list>
#include <ranges>
#include <vector>

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT InClause : public IClause {
    public:
        // Member functions

        InClause(std::string column, std::vector<binder_t> binders, bool negated);


        // Executors

        [[nodiscard]]
        ers::Status render(build_context_t& ctx) const override;


        // Misc

        [[nodiscard]]
        ClausePtr clone() const override;


    private:
        std::string _column;
        std::vector<binder_t> _binders;
        bool _negated;
    };


    namespace clauses {
        template<std::ranges::input_range R>
        ClausePtr where_in(std::string column, R&& values, bool negated = false) {
            std::vector<binder_t> binders;

            if constexpr (std::ranges::sized_range<R>)
                binders.reserve(std::ranges::size(values));

            for (auto&& value : values)
                binders.emplace_back(make_binder(std::forward<decltype(value)>(value)));

            return std::make_unique<InClause>(std::move(column), std::move(binders), negated);
        }

        template<typename T>
        ClausePtr where_in(std::string column, std::initializer_list<T> values, bool negated = false) {
            return where_in<std::initializer_list<T>>(std::move(column), values, negated);
        }
    }
}
