#include "erslib/dbio/clauses/order.hpp"

// ers
#include <erslib/dbio/clauses/raw.hpp>


dbio::impl::OrderClause::OrderClause(std::string column, Order order) :
    IClause(section::order_by),
    _column(std::move(column)),
    _order(order) {
}

ers::Status dbio::impl::OrderClause::render(build_context_t& ctx) const {
    if (!is_identifier(_column))
        return ers::make_error("dbio::Query: invalid column name '{}'", _column);

    ctx.query += _column;
    ctx.query += _order == Order::Desc ? " DESC" : " ASC";

    return ers::ok;
}

dbio::impl::ClausePtr dbio::impl::OrderClause::clone() const {
    return std::make_unique<OrderClause>(_column, _order);
}


dbio::impl::ClausePtr dbio::impl::clauses::order_by(std::string column, Order order) {
    return std::make_unique<OrderClause>(std::move(column), order);
}
dbio::impl::ClausePtr dbio::impl::clauses::order_by_random() {
    return raw(section::order_by, "RANDOM()");
}
