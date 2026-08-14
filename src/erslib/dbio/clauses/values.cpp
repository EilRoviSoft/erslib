#include "erslib/dbio/clauses/values.hpp"

// ers
#include <erslib/dbio/slots/values.hpp>


dbio::impl::ValuesClause::ValuesClause(std::vector<binder_t> binders) :
    IClause(&slots::values),
    _binders(std::move(binders)) {
}

ers::Status dbio::impl::ValuesClause::render(Context& ctx) const {
    if (_binders.empty())
        return ers::make_error("Slot '{}' needs at least one value.", slot()->name);

    bool first = true;

    for (const auto& it : _binders) {
        if (!first)
            ctx.query += ", ";
        first = false;

        ctx.query += it ? ctx.bind(it) : ctx.bind_null();
    }

    return ers::ok;
}

dbio::impl::ClausePtr dbio::impl::ValuesClause::clone() const {
    return std::make_unique<ValuesClause>(_binders);
}
