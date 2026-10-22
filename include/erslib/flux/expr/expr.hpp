#pragma once

// std
#include <concepts>
#include <initializer_list>
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
    struct Aliased;

    // Only a literal converts, so the text never comes from a runtime string.
    struct sql_text {
        std::string_view text;

        consteval sql_text(const char* literal) :
            text(literal) {
        }
    };


    // Anything with 'Expr to_expr() const' converts to Expr, a column of a table struct for example.
    template<typename T>
    concept ExprSource = requires(const T& what) {
        { what.to_expr() } -> std::same_as<Expr>;
    };

    template<typename T>
    concept ExprValue = !std::same_as<std::remove_cvref_t<T>, Expr>
        && !ExprSource<std::remove_cvref_t<T>>
        && !std::convertible_to<T, ExprNodePtr>;

    template<typename T>
    concept IsExpr = std::same_as<std::remove_cvref_t<T>, Expr> || ExprSource<std::remove_cvref_t<T>>;

    template<typename T>
    concept ExprLike = IsExpr<T> || ExprValue<T>;

    // A source can limit what it is compared with: 'template<typename U> static constexpr bool accepts'.
    template<typename A, typename B>
    concept AcceptsOperand = !requires { std::remove_cvref_t<A>::template accepts<std::decay_t<B>>; }
        || std::remove_cvref_t<A>::template accepts<std::decay_t<B>>;

    template<typename L, typename R>
    concept ExprOperands = ExprLike<L> && ExprLike<R> && (IsExpr<L> || IsExpr<R>)
        && AcceptsOperand<L, R> && AcceptsOperand<R, L>;
}


// Implementation --------------------------------------------------------------

namespace flux::impl {
    // The builder methods of an expression, for Expr and for anything with 'Expr to_expr() const':
    // derive from ExprMethods.
    class ExprMethods {
    public:
        // Generators

        Expr is_null(this const auto& self);
        Expr is_not_null(this const auto& self);
        Expr is_distinct_from(this const auto& self, Expr other);

        Expr like(this const auto& self, Expr pattern);
        Expr ilike(this const auto& self, Expr pattern);
        Expr between(this const auto& self, Expr low, Expr high);
        Expr contains(this const auto& self, Expr other);

        Expr in(this const auto& self, std::initializer_list<Expr> values);

        template<std::same_as<QueryBuilder> Q>
        Expr in(this const auto& self, Q subquery);

        template<std::ranges::input_range R>
        Expr in(this const auto& self, R&& values);

        Expr any(this const auto& self, Expr array);
        Expr cast(this const auto& self, std::string_view type);

        // For columns() and returning(): "<expr> AS <alias>".
        Aliased as(this const auto& self, std::string alias);
    };


    class ERSLIB_EXPORT Expr : public ExprMethods {
    public:
        // Member functions

        explicit Expr(ExprNodePtr node);

        template<ExprValue T>
        Expr(T&& value);

        template<ExprSource T>
        Expr(const T& source);

        Expr(const Expr&) = default;
        Expr(Expr&&) noexcept = default;

        Expr& operator=(const Expr&) & = default;
        Expr& operator=(Expr&&) & noexcept = default;


        // Observers

        EPrecedence precedence() const noexcept;
        const IExprNode& node() const noexcept { return *_node; }


        // Executors

        ers::Status render(Context& ctx, EPrecedence parent = EPrecedence::Or) const;


    private:
        ExprNodePtr _node;
    };


    struct Aliased {
        Expr expr;
        std::string alias;
    };


    // A column name or an expression, where a bare name means a column.
    struct ColumnExpr {
        Expr expr;


        template<IsExpr T>
        ColumnExpr(T&& what) :
            expr(std::forward<T>(what)) {
        }

        template<typename S>
            requires (std::convertible_to<S, std::string_view> && !IsExpr<S>)
        ColumnExpr(S&& name);
    };


    // Node factories

    template<std::derived_from<IExprNode> T, typename... Args>
    Expr make_expr(Args&&... args);

    ERSLIB_EXPORT Expr make_value(binder_t binder);
    ERSLIB_EXPORT Expr make_param(binder_t binder);
    ERSLIB_EXPORT Expr make_const(sql_text text);
    ERSLIB_EXPORT Expr make_postfix(Expr operand, sql_text suffix, EPrecedence precedence);
    ERSLIB_EXPORT Expr make_between(Expr operand, Expr low, Expr high);
    ERSLIB_EXPORT Expr make_in(Expr operand, std::vector<Expr> items);
    ERSLIB_EXPORT Expr make_in(Expr operand, QueryBuilder subquery);
    ERSLIB_EXPORT Expr make_any(Expr operand, Expr array);
    ERSLIB_EXPORT Expr make_cast(Expr operand, std::string_view type);
    ERSLIB_EXPORT Expr make_binary(std::string_view op, Expr lhs, Expr rhs, EPrecedence precedence, bool left_associative = false);
    ERSLIB_EXPORT Expr make_fn(std::string_view name, std::vector<Expr> args);
    ERSLIB_EXPORT Expr make_raw(sql_text pattern, std::vector<Expr> args);
    ERSLIB_EXPORT Expr make_row(std::vector<Expr> items);
    ERSLIB_EXPORT Expr make_logical(bool conjunction, Expr lhs, Expr rhs);
    ERSLIB_EXPORT Expr make_not(Expr what);


    namespace clauses {
        ERSLIB_EXPORT Expr col(std::string name);
        ERSLIB_EXPORT Expr col(std::string_view table, std::string_view name);
        ERSLIB_EXPORT Expr null();
        ERSLIB_EXPORT Expr sql_bool(bool value);
        ERSLIB_EXPORT Expr exists(QueryBuilder subquery);
        ERSLIB_EXPORT Expr subquery(QueryBuilder query);

        template<typename T>
        Expr val(T&& value);

        // One placeholder per statement, however many times it's used.
        template<typename T>
        Expr param(T&& value);

        template<typename... Args>
        Expr row(Args&&... items);
    }
}


// Implementation --------------------------------------------------------------

#include "expr.inl"
