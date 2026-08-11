#include "erslib/dbio/clauses/in.hpp"

dbio::InClause::InClause(std::string column, std::vector<internal::binder_t> binders, bool negated) :
    IClause(section::where),
    _column(std::move(column)),
    _binders(std::move(binders)),
    _negated(negated) {
}

ers::Status dbio::InClause::render(build_context_t& ctx) const {
    if (!internal::is_identifier(_column))
        return ers::make_error("dbio::Query: invalid column name '{}'", _column);

    // An empty set never matches, and "IN ()" is not valid SQL.
    if (_binders.empty()) {
        ctx.query += _negated ? "TRUE" : "FALSE";
        return ers::ok;
    }

    ctx.query += _column;
    ctx.query += _negated ? " NOT IN (" : " IN (";

    for (const auto& binder : _binders) {
        if (&binder != &_binders.front())
            ctx.query += ", ";

        ctx.query += ctx.bind(binder);
    }

    ctx.query += ')';

    return ers::ok;
}

dbio::ClausePtr dbio::InClause::clone() const {
    return std::make_unique<InClause>(_column, _binders, _negated);
}
