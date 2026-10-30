#pragma once

// std
#include <concepts>
#include <initializer_list>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>


// Expr

template<typename S>
    requires (std::convertible_to<S, std::string_view> && !flux::impl::IsExpr<S>)
flux::impl::ColumnExpr::ColumnExpr(S&& name) :
    expr(clauses::col(std::string(std::string_view(name)))) {
}

template<flux::impl::ExprSource T>
flux::impl::Expr::Expr(const T& source) :
    Expr(source.to_expr()) {
}

template<flux::impl::ExprValue T>
flux::impl::Expr::Expr(T&& value) :
    Expr(make_value(make_binder(std::forward<T>(value)))) {
}

// ExprMethods

namespace flux::impl {
    template<typename T>
    Expr to_expr_of(const T& what) {
        if constexpr (std::same_as<T, Expr>)
            return what;
        else
            return what.to_expr();
    }
}

flux::impl::Expr flux::impl::ExprMethods::is_null(this const auto& self) {
    return make_postfix(to_expr_of(self), " IS NULL", EPrecedence::Is);
}

flux::impl::Expr flux::impl::ExprMethods::is_not_null(this const auto& self) {
    return make_postfix(to_expr_of(self), " IS NOT NULL", EPrecedence::Is);
}

flux::impl::Expr flux::impl::ExprMethods::is_distinct_from(this const auto& self, Expr other) {
    return make_binary("IS DISTINCT FROM", to_expr_of(self), std::move(other), EPrecedence::Is);
}

flux::impl::Expr flux::impl::ExprMethods::like(this const auto& self, Expr pattern) {
    return make_binary("LIKE", to_expr_of(self), std::move(pattern), EPrecedence::Like);
}

flux::impl::Expr flux::impl::ExprMethods::ilike(this const auto& self, Expr pattern) {
    return make_binary("ILIKE", to_expr_of(self), std::move(pattern), EPrecedence::Like);
}

flux::impl::Expr flux::impl::ExprMethods::between(this const auto& self, Expr low, Expr high) {
    return make_between(to_expr_of(self), std::move(low), std::move(high));
}

flux::impl::Expr flux::impl::ExprMethods::contains(this const auto& self, Expr other) {
    return make_binary("@>", to_expr_of(self), std::move(other), EPrecedence::Other);
}

flux::impl::Expr flux::impl::ExprMethods::in(this const auto& self, std::initializer_list<Expr> values) {
    return make_in(to_expr_of(self), std::vector<Expr>(values));
}

template<std::same_as<flux::impl::QueryBuilder> Q>
flux::impl::Expr flux::impl::ExprMethods::in(this const auto& self, Q subquery) {
    return make_in(to_expr_of(self), std::move(subquery));
}

template<std::ranges::input_range R>
flux::impl::Expr flux::impl::ExprMethods::in(this const auto& self, R&& values) {
    std::vector<Expr> items;

    for (auto&& it : values)
        items.push_back(Expr(it));

    return make_in(to_expr_of(self), std::move(items));
}

flux::impl::Expr flux::impl::ExprMethods::any(this const auto& self, Expr array) {
    return make_any(to_expr_of(self), std::move(array));
}

flux::impl::Expr flux::impl::ExprMethods::cast(this const auto& self, std::string_view type) {
    return make_cast(to_expr_of(self), type);
}

flux::impl::Aliased flux::impl::ExprMethods::as(this const auto& self, std::string alias) {
    return { to_expr_of(self), std::move(alias) };
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
    (out.push_back(Expr(std::forward<Args>(items))), ...);

    return make_row(std::move(out));
}
