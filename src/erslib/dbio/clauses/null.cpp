#include "erslib/dbio/clauses/null.hpp"


dbio::impl::NullClause::NullClause(std::string column, bool is_null) :
    IClause(section::where),
    _column(std::move(column)),
    _is_null(is_null) {
}

ers::Status dbio::impl::NullClause::render(build_context_t& ctx) const {
    if (!is_identifier(_column))
        return ers::make_error("dbio::Query: invalid column name '{}'", _column);

    ctx.query += _column;
    ctx.query += _is_null ? " IS NULL" : " IS NOT NULL";

    return ers::ok;
}

dbio::impl::ClausePtr dbio::impl::NullClause::clone() const {
    return std::make_unique<NullClause>(_column, _is_null);
}


dbio::impl::ClausePtr dbio::impl::clauses::where_null(std::string column, bool is_null) {
    return std::make_unique<NullClause>(std::move(column), is_null);
}
