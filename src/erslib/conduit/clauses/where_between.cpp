#include "erslib/conduit/clauses/where_between.hpp"

// ers
#include <erslib/conduit/slots/where.hpp>


conduit::impl::WhereBetweenClause::WhereBetweenClause(std::string column, binder_t low, binder_t high) :
    IWhereClause(&slots::where),
    _column(std::move(column)),
    _low(std::move(low)),
    _high(std::move(high)) {
}

ers::Status conduit::impl::WhereBetweenClause::render(Context& ctx) const {
    if (auto s = append_identifier(ctx, _column, slot()); !s)
        return s;

    ctx.query += " BETWEEN ";
    ctx.query += ctx.bind(_low);
    ctx.query += " AND ";
    ctx.query += ctx.bind(_high);

    return ers::ok;
}
