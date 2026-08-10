#include "erslib/dbio/clauses/column.hpp"


dbio::ColumnClause::ColumnClause(std::string name) :
    IClause(Section::Column),
    _name(std::move(name)) {
}

ers::Status dbio::ColumnClause::render(build_context_t& ctx) const {
    if (!internal::is_identifier(_name))
        return ers::make_error("dbio::Query: invalid column name '{}'", _name);

    ctx.query += _name;

    return ers::ok;
}

dbio::ClausePtr dbio::ColumnClause::clone() const {
    return std::make_unique<ColumnClause>(_name);
}


dbio::ClausePtr dbio::clauses::column(std::string name) {
    return std::make_unique<ColumnClause>(std::move(name));
}
