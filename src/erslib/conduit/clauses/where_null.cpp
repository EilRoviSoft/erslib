#include "erslib/conduit/clauses/where_null.hpp"

// ers
#include <erslib/conduit/slots/where.hpp>


conduit::impl::WhereNullClause::WhereNullClause(std::string column, bool is_null) :
    IWhereClause(&slots::where),
    _column(std::move(column)),
    _is_null(is_null) {
}

ers::Status conduit::impl::WhereNullClause::render(Context& ctx) const {
    if (auto s = append_identifier(ctx, _column, slot()); !s)
        return s;

    ctx.query += _is_null ? " IS NULL" : " IS NOT NULL";

    return ers::ok;
}


conduit::impl::Clause conduit::impl::clauses::where_null(std::string column, bool is_null) {
    return make_clause<WhereNullClause>(std::move(column), is_null);
}
