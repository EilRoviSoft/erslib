#include "erslib/dbio/clauses/order.hpp"

// ers
#include <erslib/dbio/clauses/raw.hpp>
#include <erslib/dbio/impl/identity.hpp>
#include <erslib/dbio/slots/order.hpp>


// OrderClause

dbio::impl::OrderClause::OrderClause(std::string column, Order order) :
    IClause(&slots::order),
    _column(std::move(column)),
    _order(order) {
}

ers::Status dbio::impl::OrderClause::render(Context& ctx) const {
    if (!is_identifier(_column)) {
        return ers::make_error("Invalid identifier '{}' in slot '{}'.",
            _column, slot()->name);
    }

    ctx.query += _column;
    ctx.query += _order == Order::Desc ? " DESC" : " ASC";

    return ers::ok;
}


// Shortcuts

dbio::impl::ClausePtr dbio::impl::clauses::order_by(std::string column, Order order) {
    return make_clause<OrderClause>(std::move(column), order);
}

dbio::impl::ClausePtr dbio::impl::clauses::order_by_random() {
    return raw(&slots::order, "RANDOM()");
}
