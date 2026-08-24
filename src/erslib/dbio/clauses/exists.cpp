#include "erslib/dbio/clauses/exists.hpp"

// ers
#include <erslib/dbio/slots/where.hpp>


dbio::impl::ExistsClause::ExistsClause(QueryBuilder subquery, bool negated) :
    IWhereClause(&slots::where),
    _subquery(std::move(subquery)),
    _negated(negated) {
}

ers::Status dbio::impl::ExistsClause::render(Context& ctx) const {
    return append_subquery(ctx, _subquery, _negated ? "NOT EXISTS (" : "EXISTS (");
}


dbio::impl::Clause dbio::impl::clauses::where_exists(QueryBuilder subquery, bool negated) {
    return make_clause<ExistsClause>(std::move(subquery), negated);
}
