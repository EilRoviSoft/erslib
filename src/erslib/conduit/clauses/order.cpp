#include "erslib/conduit/clauses/order.hpp"

// std
#include <format>

// ers
#include <erslib/conduit/clauses/raw.hpp>
#include <erslib/conduit/slots/order.hpp>


// OrderClause

conduit::impl::OrderClause::OrderClause(std::string column, EOrder order) :
    IClause(&slots::order),
    _column(std::move(column)),
    _order(order) {
}

ers::Status conduit::impl::OrderClause::render(Context& ctx) const {
    if (auto s = append_identifier(ctx, _column, slot()); !s)
        return s;

    ctx.query += _order == EOrder::Desc ? " DESC" : " ASC";

    return ers::ok;
}


conduit::impl::Clause conduit::impl::clauses::order_by(std::string column, EOrder order) {
    return make_clause<OrderClause>(std::move(column), order);
}

conduit::impl::Clause conduit::impl::clauses::order_by_random() {
    return raw(&slots::order, "RANDOM()");
}


// StableRandomClause

conduit::impl::StableRandomClause::StableRandomClause(std::string key, i64 seed) :
    IClause(&slots::order),
    _key(std::move(key)),
    _seed(seed) {
}

ers::Status conduit::impl::StableRandomClause::render(Context& ctx) const {
    if (auto s = check_identifier(_key, slot()); !s)
        return s;

    ctx.query += std::format("md5({}::text || {}::text)",
        _key, ctx.bind(make_binder(_seed)));

    return ers::ok;
}


conduit::impl::Clause conduit::impl::clauses::order_by_stable_random(std::string key, i64 seed) {
    return make_clause<StableRandomClause>(std::move(key), seed);
}
