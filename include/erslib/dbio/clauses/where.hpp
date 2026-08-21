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
#include <erslib/core/type/general.hpp>
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
    enum class Op : u8 {
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

        template<typename T>
            requires (!std::same_as<std::remove_cvref_t<T>, Op>)
        Clause where(std::string column, T&& value) {
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


    private:
        std::string _column;
        bool _is_null;
    };


    namespace clauses {
        Clause where_null(std::string column, bool is_null = true);
    }
}
