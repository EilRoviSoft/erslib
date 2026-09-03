#include "erslib/conduit/clauses/values.hpp"

// ers
#include <erslib/conduit/slots/values.hpp>


// ValuesClause

conduit::impl::ValuesClause::ValuesClause(std::vector<binder_t> binders) :
    IClause(&slots::values),
    _binders(std::move(binders)) {
}

ers::Status conduit::impl::ValuesClause::render(Context& ctx) const {
    if (_binders.empty())
        return ers::make_error("Slot '{}' needs at least one value.", slot()->name);
    return append_binders(ctx, _binders);
}
