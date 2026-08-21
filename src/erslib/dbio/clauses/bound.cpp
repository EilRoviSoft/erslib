#include "erslib/dbio/clauses/bound.hpp"

// ers
#include <erslib/dbio/slots/limit.hpp>
#include <erslib/dbio/slots/offset.hpp>


// BoundClause

dbio::impl::BoundClause::BoundClause(SlotRef slot, i64 amount) :
    IClause(slot),
    _amount(amount) {
}

ers::Status dbio::impl::BoundClause::render(Context& ctx) const {
    if (_amount < 0)
        return ers::make_error("Negative bound '{}' is not supported.", _amount);
    ctx.query += ctx.bind(make_binder(_amount));
    return ers::ok;
}


// Shortcuts

dbio::impl::Clause dbio::impl::clauses::with_limit(i64 amount) {
    return make_clause<BoundClause>(&slots::limit, amount);
}

dbio::impl::Clause dbio::impl::clauses::with_offset(i64 amount) {
    return make_clause<BoundClause>(&slots::offset, amount);
}
