#include "erslib/dbio/clauses/bound.hpp"


dbio::BoundClause::BoundClause(Section section, int64_t amount) :
    IClause(section),
    _amount(amount) {
}

ers::Status dbio::BoundClause::render(build_context_t& ctx) const {
    if (_amount < 0)
        return ers::make_error("dbio::Query: negative bound '{}'", _amount);

    ctx.query += ctx.bind(internal::make_binder(_amount));

    return ers::ok;
}

dbio::ClausePtr dbio::BoundClause::clone() const {
    return std::make_unique<BoundClause>(section(), _amount);
}


dbio::ClausePtr dbio::clauses::limit(int64_t amount) {
    return std::make_unique<BoundClause>(Section::Limit, amount);
}

dbio::ClausePtr dbio::clauses::offset(int64_t amount) {
    return std::make_unique<BoundClause>(Section::Offset, amount);
}
