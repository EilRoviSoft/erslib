#include "erslib/flux/clauses/values.hpp"

// ers
#include <erslib/flux/slots/values.hpp>


// Implementation --------------------------------------------------------------

flux::impl::ValuesClause::ValuesClause(std::vector<binder_t> binders) :
    IClause(&slots::values),
    _binders(std::move(binders)) {
}

ers::Status flux::impl::ValuesClause::render(Context& ctx) const {
    if (_binders.empty())
        return ers::make_error("Slot '{}' needs at least one value.", slot()->name);
    return append_binders(ctx, _binders);
}
