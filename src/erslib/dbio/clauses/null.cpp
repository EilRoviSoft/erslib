#include "erslib/dbio/clauses/null.hpp"


dbio::NullClause::NullClause(std::string column, bool is_null) :
    IClause(section::where),
    _column(std::move(column)),
    _is_null(is_null) {
}

ers::Status dbio::NullClause::render(build_context_t& ctx) const {
    if (!internal::is_identifier(_column))
        return ers::make_error("dbio::Query: invalid column name '{}'", _column);

    ctx.query += _column;
    ctx.query += _is_null ? " IS NULL" : " IS NOT NULL";

    return ers::ok;
}

dbio::ClausePtr dbio::NullClause::clone() const {
    return std::make_unique<NullClause>(_column, _is_null);
}


dbio::ClausePtr dbio::clauses::where_null(std::string column, bool is_null) {
    return std::make_unique<NullClause>(std::move(column), is_null);
}
