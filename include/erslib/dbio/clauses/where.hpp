#pragma once

// std
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


// IWhere

namespace dbio::impl {
    class ERSLIB_EXPORT IWhereClause : public IClause {
    public:
        // Member functions

        IWhereClause(SlotRef slot);


        // Executors

        ers::Status render(Context& ctx) const override = 0;
    };
}


// WhereOp

namespace dbio::impl {
    enum class Op : uint8_t {
        Eq, // ==
        Ne, // !=
        Lt, // <
        Le, // <=
        Gt, // >
        Ge, // >=
        Like,
        ILike
    };


    class ERSLIB_EXPORT WhereOpClause : public IWhereClause {
    public:
        // Member functions

        WhereOpClause(std::string column, Op op, binder_t binder);


        // Executors

        ers::Status render(Context& ctx) const override;


        // Misc

        ClausePtr clone() const override;


    private:
        std::string _column;
        Op _op;
        binder_t _binder;
    };


    namespace clauses {
        template<typename T>
        ClausePtr where(std::string column, Op op, T&& value) {
            return std::make_unique<WhereOpClause>(std::move(column), op, make_binder(std::forward<T>(value)));
        }

        template<typename T>
            requires (!std::same_as<std::remove_cvref_t<T>, Op>)
        ClausePtr where(std::string column, T&& value) {
            return where(std::move(column), Op::Eq, std::forward<T>(value));
        }
    }
}


// WhereIn

namespace dbio::impl {
    class ERSLIB_EXPORT WhereInClause : public IWhereClause {
    public:
        // Member functions

        WhereInClause(std::string column, std::vector<binder_t> binders, bool negated);


        // Executors

        ers::Status render(Context& ctx) const override;


        // Misc

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

            return std::make_unique<WhereInClause>(std::move(column), std::move(binders), negated);
        }

        template<typename T>
        ClausePtr where_in(std::string column, std::initializer_list<T> values, bool negated = false) {
            return where_in<std::initializer_list<T>>(std::move(column), values, negated);
        }
    }
}


// WhereNull

namespace dbio::impl {
    class ERSLIB_EXPORT WhereNullClause : public IWhereClause {
    public:
        // Member functions

        WhereNullClause(std::string column, bool is_null);


        // Executors

        ers::Status render(Context& ctx) const override;


        // Misc

        ClausePtr clone() const override;


    private:
        std::string _column;
        bool _is_null;
    };


    namespace clauses {
        ClausePtr where_null(std::string column, bool is_null = true);
    }
}
