#include "erslib/flux/expr/aggregate.hpp"

// ers
#include "nodes/aggregate.inl"


// Implementation --------------------------------------------------------------

flux::impl::Expr flux::impl::make_aggregate(std::string_view name, aggregate_t what) {
    return make_expr<nodes::AggregateNode>(name, std::move(what));
}

std::vector<flux::impl::Expr> flux::impl::to_exprs(std::vector<ColumnExpr> items) {
    std::vector<Expr> out;
    out.reserve(items.size());

    for (auto& it : items)
        out.push_back(std::move(it.expr));

    return out;
}


// Shortcuts

flux::impl::Expr flux::impl::clauses::count_all() {
    return make_aggregate("count", { .args = { make_const("*") } });
}

flux::impl::Expr flux::impl::clauses::count_of(ColumnExpr what) {
    return agg<"count">(std::move(what));
}

flux::impl::Expr flux::impl::clauses::count_distinct(ColumnExpr what) {
    return agg_distinct<"count">(std::move(what));
}

flux::impl::Expr flux::impl::clauses::min_of(ColumnExpr what) {
    return agg<"min">(std::move(what));
}

flux::impl::Expr flux::impl::clauses::max_of(ColumnExpr what) {
    return agg<"max">(std::move(what));
}

flux::impl::Expr flux::impl::clauses::sum_of(ColumnExpr what) {
    return agg<"sum">(std::move(what));
}

flux::impl::Expr flux::impl::clauses::avg_of(ColumnExpr what) {
    return agg<"avg">(std::move(what));
}

flux::impl::Expr flux::impl::clauses::array_agg_of(ColumnExpr what) {
    return agg<"array_agg">(std::move(what));
}

flux::impl::Expr flux::impl::clauses::bool_and_of(ColumnExpr what) {
    return agg<"bool_and">(std::move(what));
}

flux::impl::Expr flux::impl::clauses::bool_or_of(ColumnExpr what) {
    return agg<"bool_or">(std::move(what));
}

flux::impl::Expr flux::impl::clauses::stddev_of(ColumnExpr what) {
    return agg<"stddev">(std::move(what));
}

flux::impl::Expr flux::impl::clauses::variance_of(ColumnExpr what) {
    return agg<"variance">(std::move(what));
}

flux::impl::Expr flux::impl::clauses::corr_of(ColumnExpr y, ColumnExpr x) {
    return agg<"corr">(std::move(y), std::move(x));
}

flux::impl::Expr flux::impl::clauses::string_agg(ColumnExpr what, Expr delimiter) {
    return make_aggregate("string_agg", { .args = { std::move(what.expr), std::move(delimiter) } });
}

flux::impl::Expr flux::impl::clauses::percentile_cont(double fraction, std::vector<ColumnExpr> order_by) {
    return make_aggregate("percentile_cont", {
        .args         = { Expr(fraction) },
        .ordered_set  = true,
        .within_group = to_exprs(std::move(order_by)),
    });
}

flux::impl::Expr flux::impl::clauses::percentile_disc(double fraction, std::vector<ColumnExpr> order_by) {
    return make_aggregate("percentile_disc", {
        .args         = { Expr(fraction) },
        .ordered_set  = true,
        .within_group = to_exprs(std::move(order_by)),
    });
}

flux::impl::Expr flux::impl::clauses::mode_of(std::vector<ColumnExpr> order_by) {
    return make_aggregate("mode", {
        .ordered_set  = true,
        .within_group = to_exprs(std::move(order_by)),
    });
}

flux::impl::Expr flux::impl::clauses::json_rows(std::string table, std::vector<ColumnExpr> order_by) {
    return make_aggregate("json_agg", {
        .args     = { make_fn("row_to_json", { col(std::move(table)) }) },
        .order_by = to_exprs(std::move(order_by)),
    });
}
