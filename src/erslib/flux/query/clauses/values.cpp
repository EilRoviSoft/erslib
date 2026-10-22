#include "erslib/flux/query/clauses/values.hpp"

// ers
#include <erslib/flux/query/slots/values.hpp>


// Implementation --------------------------------------------------------------

flux::impl::ValuesClause::ValuesClause(std::vector<Expr> items) :
    IClause(&slots::values),
    _items(std::move(items)) {
}

ers::Status flux::impl::ValuesClause::render(Context& ctx) const {
    if (_items.empty())
        return ers::make_error("Slot '{}' needs at least one value.", slot()->name);

    bool first = true;

    for (const auto& it : _items) {
        if (!first)
            ctx.query += ", ";
        first = false;

        ERS_TRY(it.render(ctx));
    }

    return ers::ok;
}
