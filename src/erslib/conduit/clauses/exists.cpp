#include "erslib/conduit/clauses/exists.hpp"

// ers
#include <erslib/conduit/slots/where.hpp>


conduit::impl::ExistsClause::ExistsClause(QueryBuilder subquery, bool negated) :
    IWhereClause(&slots::where),
    _subquery(std::move(subquery)),
    _negated(negated) {
}

ers::Status conduit::impl::ExistsClause::render(Context& ctx) const {
    return append_subquery(ctx, _subquery, _negated ? "NOT EXISTS (" : "EXISTS (");
}


conduit::impl::Clause conduit::impl::clauses::where_exists(QueryBuilder subquery, bool negated) {
    return make_clause<ExistsClause>(std::move(subquery), negated);
}
