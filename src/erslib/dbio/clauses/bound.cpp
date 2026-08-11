#include "erslib/dbio/clauses/bound.hpp"


dbio::impl::BoundClause::BoundClause(Section section, int64_t amount) :
    IClause(section),
    _amount(amount) {
}

ers::Status dbio::impl::BoundClause::render(build_context_t& ctx) const {
    if (_amount < 0)
        return ers::make_error("dbio::Query: negative bound '{}'", _amount);

    ctx.query += ctx.bind(make_binder(_amount));

    return ers::ok;
}

dbio::impl::ClausePtr dbio::impl::BoundClause::clone() const {
    return std::make_unique<BoundClause>(section(), _amount);
}


dbio::impl::ClausePtr dbio::impl::clauses::with_limit(int64_t amount) {
    return std::make_unique<BoundClause>(section::limit, amount);
}

dbio::impl::ClausePtr dbio::impl::clauses::with_offset(int64_t amount) {
    return std::make_unique<BoundClause>(section::offset, amount);
}
