#include "erslib/dbio/clauses/order.hpp"

// std
#include <format>

// ers
#include <erslib/dbio/clauses/raw.hpp>
#include <erslib/dbio/slots/order.hpp>


// OrderClause

dbio::impl::OrderClause::OrderClause(std::string column, Order order) :
    IClause(&slots::order),
    _column(std::move(column)),
    _order(order) {
}

ers::Status dbio::impl::OrderClause::render(Context& ctx) const {
    if (auto s = append_identifier(ctx, _column, slot()); !s)
        return s;

    ctx.query += _order == Order::Desc ? " DESC" : " ASC";

    return ers::ok;
}


dbio::impl::Clause dbio::impl::clauses::order_by(std::string column, Order order) {
    return make_clause<OrderClause>(std::move(column), order);
}

dbio::impl::Clause dbio::impl::clauses::order_by_random() {
    return raw(&slots::order, "RANDOM()");
}


// StableRandomClause

dbio::impl::StableRandomClause::StableRandomClause(std::string key, i64 seed) :
    IClause(&slots::order),
    _key(std::move(key)),
    _seed(seed) {
}

ers::Status dbio::impl::StableRandomClause::render(Context& ctx) const {
    if (auto s = check_identifier(_key, slot()); !s)
        return s;

    ctx.query += std::format("md5({}::text || {}::text)",
        _key, ctx.bind(make_binder(_seed)));

    return ers::ok;
}


dbio::impl::Clause dbio::impl::clauses::order_by_stable_random(std::string key, i64 seed) {
    return make_clause<StableRandomClause>(std::move(key), seed);
}
