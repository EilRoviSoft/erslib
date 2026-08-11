#include "erslib/dbio/clauses/raw.hpp"


dbio::impl::RawClause::RawClause(Section section, std::string sql, std::vector<binder_t> binders) :
    IClause(section),
    _sql(std::move(sql)),
    _binders(std::move(binders)) {
}

ers::Status dbio::impl::RawClause::render(build_context_t& ctx) const {
    size_t index = 0;

    for (const char c : _sql) {
        if (c != '?') {
            ctx.query += c;
            continue;
        }

        if (index >= _binders.size())
            return ers::make_error("dbio::Query: raw clause '{}' has more markers than values", _sql);

        ctx.query += ctx.bind(_binders[index++]);
    }

    if (index != _binders.size())
        return ers::make_error("dbio::Query: raw clause '{}' has fewer markers than values", _sql);

    return ers::ok;
}

dbio::impl::ClausePtr dbio::impl::RawClause::clone() const {
    return std::make_unique<RawClause>(section(), _sql, _binders);
}
