#pragma once

// std
#include <concepts>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// ers
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/flux/expr/expr.hpp>

// export
#include <erslib/export.hpp>


// Details ---------------------------------------------------------------------

namespace flux::impl {
    // fn([DISTINCT] args [ORDER BY order_by]) [WITHIN GROUP (ORDER BY within_group)]
    struct aggregate_t {
        std::vector<Expr> args;
        bool distinct = false;
        std::vector<Expr> order_by;
        bool ordered_set = false;
        std::vector<Expr> within_group;
    };


    // The name is kept as a view: a literal or a template argument.
    ERSLIB_EXPORT Expr make_aggregate(std::string_view name, aggregate_t what);

    ERSLIB_EXPORT std::vector<Expr> to_exprs(std::vector<ColumnExpr> items);
}


// Implementation --------------------------------------------------------------

namespace flux::impl::clauses {
    // For anything without a shorthand: agg<"regr_slope">("y", "x").
    template<ers::fixed_string Fn, typename... Args>
        requires (std::constructible_from<ColumnExpr, Args> && ...)
    Expr agg(Args&&... args) {
        return make_aggregate(Fn.to_sv(), { .args = { ColumnExpr(std::forward<Args>(args)).expr... } });
    }

    template<ers::fixed_string Fn>
    Expr agg_distinct(ColumnExpr what) {
        return make_aggregate(Fn.to_sv(), { .args = { std::move(what.expr) }, .distinct = true });
    }


    ERSLIB_EXPORT Expr count_all();
    ERSLIB_EXPORT Expr count_of(ColumnExpr what);
    ERSLIB_EXPORT Expr count_distinct(ColumnExpr what);

    ERSLIB_EXPORT Expr min_of(ColumnExpr what);
    ERSLIB_EXPORT Expr max_of(ColumnExpr what);
    ERSLIB_EXPORT Expr sum_of(ColumnExpr what);
    ERSLIB_EXPORT Expr avg_of(ColumnExpr what);

    ERSLIB_EXPORT Expr array_agg_of(ColumnExpr what);
    ERSLIB_EXPORT Expr bool_and_of(ColumnExpr what);
    ERSLIB_EXPORT Expr bool_or_of(ColumnExpr what);

    ERSLIB_EXPORT Expr stddev_of(ColumnExpr what);
    ERSLIB_EXPORT Expr variance_of(ColumnExpr what);

    ERSLIB_EXPORT Expr corr_of(ColumnExpr y, ColumnExpr x);

    // The delimiter is a value.
    ERSLIB_EXPORT Expr string_agg(ColumnExpr what, Expr delimiter);

    ERSLIB_EXPORT Expr percentile_cont(double fraction, std::vector<ColumnExpr> order_by);
    ERSLIB_EXPORT Expr percentile_disc(double fraction, std::vector<ColumnExpr> order_by);
    ERSLIB_EXPORT Expr mode_of(std::vector<ColumnExpr> order_by);

    // json_agg(row_to_json(<table>) ORDER BY ...)
    ERSLIB_EXPORT Expr json_rows(std::string table, std::vector<ColumnExpr> order_by = {});
}
