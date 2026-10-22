#pragma once

// std
#include <concepts>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>


// Expr

template<flux::impl::ExprValue T>
flux::impl::Expr::Expr(T&& value) :
    Expr(make_value(make_binder(std::forward<T>(value)))) {
}

template<std::ranges::input_range R>
flux::impl::Expr flux::impl::Expr::in(R&& values) const {
    std::vector<Expr> items;

    for (auto&& it : values)
        items.emplace_back(Expr(it));

    return _in(std::move(items));
}


// Node factories

template<std::derived_from<flux::impl::IExprNode> T, typename... Args>
flux::impl::Expr flux::impl::make_expr(Args&&... args) {
    return Expr(std::make_shared<const T>(std::forward<Args>(args)...));
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

    // A template, so 'Result' and friends never reach it through Expr's converting constructor.
    template<std::same_as<Expr> T>
    Expr operator!(const T& what) {
        return make_not(what);
    }
}


// Shortcuts

template<typename T>
flux::impl::Expr flux::impl::clauses::val(T&& value) {
    return Expr(std::forward<T>(value));
}

template<typename T>
flux::impl::Expr flux::impl::clauses::param(T&& value) {
    return make_param(make_binder(std::forward<T>(value)));
}

template<typename... Args>
flux::impl::Expr flux::impl::clauses::row(Args&&... items) {
    std::vector<Expr> out;
    out.reserve(sizeof...(Args));
    (out.emplace_back(Expr(std::forward<Args>(items))), ...);

    return make_row(std::move(out));
}
