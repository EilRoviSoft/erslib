#include "erslib/flux/query/clauses/exists.hpp"

// ers
#include <erslib/flux/query/slots/where.hpp>


flux::impl::ExistsClause::ExistsClause(QueryBuilder subquery, bool negated) :
    IWhereClause(&slots::where),
    _subquery(std::move(subquery)),
    _negated(negated) {
}

ers::Status flux::impl::ExistsClause::render(Context& ctx) const {
    return append_subquery(ctx, _subquery, _negated ? "NOT EXISTS (" : "EXISTS (");
}


flux::impl::Clause flux::impl::clauses::where_exists(QueryBuilder subquery, bool negated) {
    return make_clause<ExistsClause>(std::move(subquery), negated);
}
