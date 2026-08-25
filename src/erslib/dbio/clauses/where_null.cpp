#include "erslib/dbio/clauses/where_null.hpp"

// ers
#include <erslib/dbio/slots/where.hpp>


dbio::impl::WhereNullClause::WhereNullClause(std::string column, bool is_null) :
    IWhereClause(&slots::where),
    _column(std::move(column)),
    _is_null(is_null) {
}

ers::Status dbio::impl::WhereNullClause::render(Context& ctx) const {
    if (auto s = append_identifier(ctx, _column, slot()); !s)
        return s;

    ctx.query += _is_null ? " IS NULL" : " IS NOT NULL";

    return ers::ok;
}


dbio::impl::Clause dbio::impl::clauses::where_null(std::string column, bool is_null) {
    return make_clause<WhereNullClause>(std::move(column), is_null);
}
