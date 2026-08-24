#include "erslib/dbio/clauses/exists.hpp"

// ers
#include <erslib/dbio/slots/where.hpp>


using std::string_literals::operator ""s;


dbio::impl::ExistsClause::ExistsClause(QueryBuilder subquery, bool negated) :
    IWhereClause(&slots::where),
    _subquery(std::move(subquery)),
    _negated(negated) {
}

ers::Status dbio::impl::ExistsClause::render(Context& ctx) const {
    ctx.query += _negated ? "NOT EXISTS (" : "EXISTS (";

    if (auto s = _subquery.build(ctx); !s)
        return s;

    ctx.query += ')';

    return ers::ok;
}


dbio::impl::Clause dbio::impl::clauses::where_exists(QueryBuilder subquery, bool negated) {
    return make_clause<ExistsClause>(std::move(subquery), negated);
}
