#include "erslib/flux/query/clauses/where_null.hpp"

// std
#include <utility>

// ers
#include <erslib/flux/query/render.hpp>
#include <erslib/flux/query/slots/where.hpp>


flux::impl::WhereNullClause::WhereNullClause(std::string column, bool is_null) :
    IWhereClause(&slots::where),
    _column(std::move(column)),
    _is_null(is_null) {
}

ers::Status flux::impl::WhereNullClause::render(Context& ctx) const {
    if (auto s = append_identifier(ctx, _column, slot()); !s)
        return s;

    ctx.query += _is_null ? " IS NULL" : " IS NOT NULL";

    return ers::ok;
}


flux::impl::Clause flux::impl::clauses::where_null(std::string column, bool is_null) {
    return make_clause<WhereNullClause>(std::move(column), is_null);
}
