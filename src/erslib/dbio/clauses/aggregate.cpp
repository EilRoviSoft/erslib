#include "erslib/dbio/clauses/aggregate.hpp"

// ers
#include <erslib/dbio/slots/column.hpp>


// Internal

namespace {
    using namespace dbio::impl;

    ers::Status append_columns(Context& ctx, const std::vector<std::string>& columns, SlotRef slot) {
        return append_joined(ctx, columns, ", ",
            [slot](Context& ctx, const std::string& name) {
                return append_identifier(ctx, name, slot);
            }
        );
    }
}


// AggregateClause

AggregateClause::AggregateClause(std::string_view fn, std::vector<std::string> columns, bool distinct) :
    IClause(&slots::column),
    _fn(fn),
    _columns(std::move(columns)),
    _distinct(distinct) {
}

ers::Status AggregateClause::render(Context& ctx) const {
    // count(*) is the only aggregate that takes a bare star.
    if (_columns.empty() && _fn != "count") {
        return ers::make_error("Aggregate '{}' needs at least one column in slot '{}'.",
            _fn, slot()->name);
    }

    ctx.query += _fn;
    ctx.query += '(';

    if (_distinct)
        ctx.query += "DISTINCT ";

    if (_columns.empty())
        ctx.query += '*';
    else if (auto s = append_columns(ctx, _columns, slot()); !s)
        return s;

    ctx.query += ')';

    return ers::ok;
}


// Shortcuts

Clause clauses::count_all() {
    return agg<"count">();
}

Clause clauses::count_of(std::string column) {
    return agg<"count">(std::move(column));
}

Clause clauses::count_distinct(std::string column) {
    return agg_distinct<"count">(std::move(column));
}

Clause clauses::min_of(std::string column) {
    return agg<"min">(std::move(column));
}

Clause clauses::max_of(std::string column) {
    return agg<"max">(std::move(column));
}

Clause clauses::sum_of(std::string column) {
    return agg<"sum">(std::move(column));
}

Clause clauses::avg_of(std::string column) {
    return agg<"avg">(std::move(column));
}

Clause clauses::array_agg_of(std::string column) {
    return agg<"array_agg">(std::move(column));
}

Clause clauses::bool_and_of(std::string column) {
    return agg<"bool_and">(std::move(column));
}

Clause clauses::bool_or_of(std::string column) {
    return agg<"bool_or">(std::move(column));
}

Clause clauses::stddev_of(std::string column) {
    return agg<"stddev">(std::move(column));
}

Clause clauses::variance_of(std::string column) {
    return agg<"variance">(std::move(column));
}

Clause clauses::corr_of(std::string y, std::string x) {
    return agg<"corr">(std::move(y), std::move(x));
}


// StringAggClause

StringAggClause::StringAggClause(std::string column, binder_t delimiter) :
    IClause(&slots::column),
    _column(std::move(column)),
    _delimiter(std::move(delimiter)) {
}

ers::Status StringAggClause::render(Context& ctx) const {
    ctx.query += "string_agg(";

    if (auto s = append_identifier(ctx, _column, slot()); !s)
        return s;

    ctx.query += ", ";
    ctx.query += ctx.bind(_delimiter);
    ctx.query += ')';

    return ers::ok;
}


Clause clauses::string_agg(std::string column, std::string delimiter) {
    return make_clause<StringAggClause>(std::move(column), make_binder(std::move(delimiter)));
}


// OrderedSetAggClause

OrderedSetAggClause::OrderedSetAggClause(
    std::string_view fn, ers::optional<binder_t> arg, std::vector<std::string> order_by
) :
    IClause(&slots::column),
    _fn(fn),
    _arg(std::move(arg)),
    _order_by(std::move(order_by)) {
}

ers::Status OrderedSetAggClause::render(Context& ctx) const {
    if (_order_by.empty()) {
        return ers::make_error("Ordered-set aggregate '{}' needs a WITHIN GROUP ordering in slot '{}'.",
            _fn, slot()->name);
    }

    ctx.query += _fn;
    ctx.query += '(';

    if (_arg)
        ctx.query += ctx.bind(*_arg);

    ctx.query += ") WITHIN GROUP (ORDER BY ";

    if (auto s = append_columns(ctx, _order_by, slot()); !s)
        return s;

    ctx.query += ')';

    return ers::ok;
}


Clause clauses::percentile_cont(double fraction, std::vector<std::string> order_by) {
    return make_clause<OrderedSetAggClause>("percentile_cont", make_binder(fraction), std::move(order_by));
}

Clause clauses::percentile_disc(double fraction, std::vector<std::string> order_by) {
    return make_clause<OrderedSetAggClause>("percentile_disc", make_binder(fraction), std::move(order_by));
}

Clause clauses::mode_of(std::vector<std::string> order_by) {
    return make_clause<OrderedSetAggClause>("mode", ers::nullopt, std::move(order_by));
}


// JsonRowsClause

JsonRowsClause::JsonRowsClause(std::string table, std::vector<std::string> order_by) :
    IClause(&slots::column),
    _table(std::move(table)),
    _order_by(std::move(order_by)) {
}

ers::Status JsonRowsClause::render(Context& ctx) const {
    if (auto s = check_identifier(_table, slot()); !s)
        return s;

    ctx.query += "json_agg(row_to_json(";
    ctx.query += _table;
    ctx.query += ')';

    if (!_order_by.empty()) {
        ctx.query += " ORDER BY ";

        if (auto s = append_columns(ctx, _order_by, slot()); !s)
            return s;
    }

    ctx.query += ')';

    return ers::ok;
}


Clause clauses::json_rows(std::string table, std::vector<std::string> order_by) {
    return make_clause<JsonRowsClause>(std::move(table), std::move(order_by));
}
