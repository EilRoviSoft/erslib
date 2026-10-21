#include "erslib/flux/query/clauses/where_expr.hpp"

// ers
#include <erslib/flux/query/slots/where.hpp>


flux::impl::WhereExprClause::WhereExprClause(Expr condition) :
    IWhereClause(&slots::where),
    _condition(std::move(condition)) {
}

ers::Status flux::impl::WhereExprClause::render(Context& ctx) const {
    return _condition.render(ctx, EPrecedence::And);
}
