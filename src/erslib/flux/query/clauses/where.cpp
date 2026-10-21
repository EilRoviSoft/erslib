#include "erslib/flux/query/clauses/where.hpp"

// ers
#include <erslib/flux/query/slots/where.hpp>


flux::impl::WhereClause::WhereClause(Expr condition) :
    IClause(&slots::where),
    _condition(std::move(condition)) {
}

ers::Status flux::impl::WhereClause::render(Context& ctx) const {
    return _condition.render(ctx, EPrecedence::And);
}
