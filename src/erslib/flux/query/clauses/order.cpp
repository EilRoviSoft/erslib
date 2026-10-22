#include "erslib/flux/query/clauses/order.hpp"

// ers
#include <erslib/flux/expr/functions.hpp>
#include <erslib/flux/query/slots/order.hpp>


// Implementation --------------------------------------------------------------

flux::impl::OrderClause::OrderClause(Expr what, EOrder order) :
    IClause(&slots::order),
    _what(std::move(what)),
    _order(order) {
}

ers::Status flux::impl::OrderClause::render(Context& ctx) const {
    ERS_TRY(_what.render(ctx));

    ctx.query += _order == EOrder::Desc ? " DESC" : " ASC";
    return ers::ok;
}


// Shortcuts

flux::impl::Clause flux::impl::clauses::order_by(ColumnExpr what, EOrder order) {
    return make_clause<OrderClause>(std::move(what.expr), order);
}

flux::impl::Clause flux::impl::clauses::order_by_random() {
    return order_by(fn_t<"random"> {}());
}

flux::impl::Clause flux::impl::clauses::order_by_stable_random(ColumnExpr key, i64 seed) {
    return order_by(fn_t<"md5"> {}(concat(key.expr.cast("text"), Expr(seed).cast("text"))));
}
