#include "erslib/flux/query/clauses/where_in.hpp"

// ers
#include <erslib/flux/query/render.hpp>
#include <erslib/flux/query/slots/where.hpp>


flux::impl::WhereInClause::WhereInClause(std::string column, std::vector<binder_t> binders, bool negated) :
    IWhereClause(&slots::where),
    _column(std::move(column)),
    _binders(std::move(binders)),
    _negated(negated) {
}

ers::Status flux::impl::WhereInClause::render(Context& ctx) const {
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
