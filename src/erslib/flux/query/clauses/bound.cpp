#include "erslib/flux/query/clauses/bound.hpp"

// ers
#include <erslib/flux/query/slots/limit.hpp>
#include <erslib/flux/query/slots/offset.hpp>


// Implementation --------------------------------------------------------------

flux::impl::BoundClause::BoundClause(SlotRef slot, i64 amount) :
    IClause(slot),
    _amount(amount) {
}

ers::Status flux::impl::BoundClause::render(Context& ctx) const {
    if (_amount < 0)
        return ers::make_error("Negative bound '{}' is not supported.", _amount);
    ctx.query += ctx.bind(make_binder(_amount));
    return ers::ok;
}


// Shortcuts

flux::impl::Clause flux::impl::clauses::with_limit(i64 amount) {
    return make_clause<BoundClause>(&slots::limit, amount);
}

flux::impl::Clause flux::impl::clauses::with_offset(i64 amount) {
    return make_clause<BoundClause>(&slots::offset, amount);
}
