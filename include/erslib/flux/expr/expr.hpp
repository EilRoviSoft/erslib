#pragma once

// std
#include <concepts>
#include <initializer_list>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// ers
#include <erslib/flux/expr/node.hpp>
#include <erslib/flux/sql/value.hpp>

// export
#include <erslib/export.hpp>


// Details ---------------------------------------------------------------------

namespace flux::impl {
    class Expr;
    class QueryBuilder;

    // Only a literal converts, so the text never comes from a runtime string.
    struct sql_text {
        std::string_view text;

        consteval sql_text(const char* literal) :
            text(literal) {
        }
    };


    ERSLIB_EXPORT ExprNodePtr make_value_node(binder_t binder);
    ERSLIB_EXPORT ExprNodePtr make_param_node(binder_t binder);


    template<typename T>
    concept ExprValue = !std::same_as<std::remove_cvref_t<T>, Expr>
        && !std::convertible_to<T, ExprNodePtr>;

    template<typename T>
    concept ExprLike = std::same_as<std::remove_cvref_t<T>, Expr> || ExprValue<T>;

    template<typename L, typename R>
    concept ExprOperands = ExprLike<L> && ExprLike<R>
        && (std::same_as<std::remove_cvref_t<L>, Expr> || std::same_as<std::remove_cvref_t<R>, Expr>);
}


// Implementation --------------------------------------------------------------

namespace flux::impl {
    class ERSLIB_EXPORT Expr {
    public:
        // Member functions

        explicit Expr(ExprNodePtr node);

        template<ExprValue T>
        Expr(T&& value) :
            Expr(make_value_node(make_binder(std::forward<T>(value)))) {
        }

        Expr(const Expr&) = default;
        Expr(Expr&&) noexcept = default;

        Expr& operator=(const Expr&) & = default;
        Expr& operator=(Expr&&) & noexcept = default;


        // Observers

        EPrecedence precedence() const noexcept;
        const IExprNode& node() const noexcept { return *_node; }


        // Executors

        ers::Status render(Context& ctx, EPrecedence parent = EPrecedence::Or) const;


        // Generators

        Expr is_null() const;
        Expr is_not_null() const;
        Expr is_distinct_from(Expr other) const;

        Expr like(Expr pattern) const;
        Expr ilike(Expr pattern) const;
        Expr between(Expr low, Expr high) const;
        Expr contains(Expr other) const;

        Expr in(std::initializer_list<Expr> values) const;
        Expr in(QueryBuilder subquery) const;

        template<std::ranges::input_range R>
        Expr in(R&& values) const {
            std::vector<Expr> items;

            for (auto&& it : values)
                items.emplace_back(Expr(it));

            return _in(std::move(items));
        }

        Expr any(Expr array) const;
        Expr cast(std::string_view type) const;


    private:
        ExprNodePtr _node;


        Expr _in(std::vector<Expr> items) const;
    };


    // Node factories

    ERSLIB_EXPORT Expr make_const(sql_text text);
    ERSLIB_EXPORT Expr make_binary(std::string_view op, Expr lhs, Expr rhs, EPrecedence precedence, bool left_associative = false);
    ERSLIB_EXPORT Expr make_fn(std::string_view name, std::vector<Expr> args);
    ERSLIB_EXPORT Expr make_raw(sql_text pattern, std::vector<Expr> args);
    ERSLIB_EXPORT Expr make_row(std::vector<Expr> items);
    ERSLIB_EXPORT Expr make_logical(bool conjunction, Expr lhs, Expr rhs);
    ERSLIB_EXPORT Expr make_not(Expr what);
}


// Operators

namespace flux::impl {
    template<typename L, typename R>
        requires ExprOperands<L, R>
    Expr operator==(L&& lhs, R&& rhs) {
        return make_binary("=", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison);
    }

    template<typename L, typename R>
        requires ExprOperands<L, R>
    Expr operator!=(L&& lhs, R&& rhs) {
        return make_binary("<>", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison);
    }

    template<typename L, typename R>
        requires ExprOperands<L, R>
    Expr operator<(L&& lhs, R&& rhs) {
        return make_binary("<", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison);
    }

    template<typename L, typename R>
        requires ExprOperands<L, R>
    Expr operator<=(L&& lhs, R&& rhs) {
        return make_binary("<=", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison);
    }

    template<typename L, typename R>
        requires ExprOperands<L, R>
    Expr operator>(L&& lhs, R&& rhs) {
        return make_binary(">", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison);
    }

    template<typename L, typename R>
        requires ExprOperands<L, R>
    Expr operator>=(L&& lhs, R&& rhs) {
        return make_binary(">=", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison);
    }


    template<typename L, typename R>
        requires ExprOperands<L, R>
    Expr operator+(L&& lhs, R&& rhs) {
        return make_binary("+", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::AddSub, true);
    }

    template<typename L, typename R>
        requires ExprOperands<L, R>
    Expr operator-(L&& lhs, R&& rhs) {
        return make_binary("-", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::AddSub, true);
    }

    template<typename L, typename R>
        requires ExprOperands<L, R>
    Expr operator*(L&& lhs, R&& rhs) {
        return make_binary("*", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::MulDiv, true);
    }

    template<typename L, typename R>
        requires ExprOperands<L, R>
    Expr operator/(L&& lhs, R&& rhs) {
        return make_binary("/", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::MulDiv, true);
    }

    template<typename L, typename R>
        requires ExprOperands<L, R>
    Expr operator%(L&& lhs, R&& rhs) {
        return make_binary("%", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::MulDiv, true);
    }


    template<typename L, typename R>
        requires ExprOperands<L, R>
    Expr operator&&(L&& lhs, R&& rhs) {
        return make_logical(true, Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)));
    }

    template<typename L, typename R>
        requires ExprOperands<L, R>
    Expr operator||(L&& lhs, R&& rhs) {
        return make_logical(false, Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)));
    }

    template<std::same_as<Expr> T>
    Expr operator!(const T& what) {
        return make_not(what);
    }
}


// Shortcuts

namespace flux::impl::clauses {
    ERSLIB_EXPORT Expr col(std::string name);
    ERSLIB_EXPORT Expr col(std::string_view table, std::string_view name);
    ERSLIB_EXPORT Expr null();
    ERSLIB_EXPORT Expr exists(QueryBuilder subquery);
    ERSLIB_EXPORT Expr subquery_of(QueryBuilder subquery);

    template<typename T>
    Expr val(T&& value) {
        return Expr(std::forward<T>(value));
    }

    // One placeholder per statement, however many times it's used.
    template<typename T>
    Expr param(T&& value) {
        return Expr(make_param_node(make_binder(std::forward<T>(value))));
    }

    template<typename... Args>
    Expr row(Args&&... items) {
        std::vector<Expr> out;
        out.reserve(sizeof...(Args));
        (out.emplace_back(Expr(std::forward<Args>(items))), ...);
        return make_row(std::move(out));
    }
}

