#include "erslib/dbio/clauses/where_in.hpp"

// ers
#include <erslib/dbio/slots/where.hpp>


dbio::impl::WhereInClause::WhereInClause(std::string column, std::vector<binder_t> binders, bool negated) :
    IWhereClause(&slots::where),
    _column(std::move(column)),
    _binders(std::move(binders)),
    _negated(negated) {
}

ers::Status dbio::impl::WhereInClause::render(Context& ctx) const {
    if (auto s = check_identifier(_column, slot()); !s)
        return s;

    if (_binders.empty()) {
        ctx.query += _negated ? "TRUE" : "FALSE";
        return ers::ok;
    }

    ctx.query += _column;
    ctx.query += _negated ? " NOT IN (" : " IN (";

    if (auto s = append_binders(ctx, _binders); !s)
        return s;

    ctx.query += ')';

    return ers::ok;
}
