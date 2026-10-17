#include "erslib/flux/clauses/order.hpp"

// std
#include <format>

// ers
#include <erslib/flux/clauses/raw.hpp>
#include <erslib/flux/slots/order.hpp>


// Implementation --------------------------------------------------------------

// OrderClause

flux::impl::OrderClause::OrderClause(std::string column, EOrder order) :
    IClause(&slots::order),
    _column(std::move(column)),
    _order(order) {
}

ers::Status flux::impl::OrderClause::render(Context& ctx) const {
    if (auto s = append_identifier(ctx, _column, slot()); !s)
        return s;

    ctx.query += _order == EOrder::Desc ? " DESC" : " ASC";

    return ers::ok;
}


flux::impl::Clause flux::impl::clauses::order_by(std::string column, EOrder order) {
    return make_clause<OrderClause>(std::move(column), order);
}

flux::impl::Clause flux::impl::clauses::order_by_random() {
    return raw(&slots::order, "RANDOM()");
}


// StableRandomClause

flux::impl::StableRandomClause::StableRandomClause(std::string key, i64 seed) :
    IClause(&slots::order),
    _key(std::move(key)),
    _seed(seed) {
}

ers::Status flux::impl::StableRandomClause::render(Context& ctx) const {
    if (auto s = check_identifier(_key, slot()); !s)
        return s;

    ctx.query += std::format("md5({}::text || {}::text)",
        _key, ctx.bind(make_binder(_seed)));

    return ers::ok;
}


flux::impl::Clause flux::impl::clauses::order_by_stable_random(std::string key, i64 seed) {
    return make_clause<StableRandomClause>(std::move(key), seed);
}
