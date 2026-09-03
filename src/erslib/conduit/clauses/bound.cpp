#include "erslib/conduit/clauses/bound.hpp"

// ers
#include <erslib/conduit/slots/limit.hpp>
#include <erslib/conduit/slots/offset.hpp>


// BoundClause

conduit::impl::BoundClause::BoundClause(SlotRef slot, i64 amount) :
    IClause(slot),
    _amount(amount) {
}

ers::Status conduit::impl::BoundClause::render(Context& ctx) const {
    if (_amount < 0)
        return ers::make_error("Negative bound '{}' is not supported.", _amount);
    ctx.query += ctx.bind(make_binder(_amount));
    return ers::ok;
}


// Shortcuts

conduit::impl::Clause conduit::impl::clauses::with_limit(i64 amount) {
    return make_clause<BoundClause>(&slots::limit, amount);
}

conduit::impl::Clause conduit::impl::clauses::with_offset(i64 amount) {
    return make_clause<BoundClause>(&slots::offset, amount);
}
