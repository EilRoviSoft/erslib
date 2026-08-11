#include "erslib/dbio/clauses/values.hpp"


dbio::impl::ValuesClause::ValuesClause(std::vector<binder_t> binders) :
    IClause(section::values),
    _binders(std::move(binders)) {
}

ers::Status dbio::impl::ValuesClause::render(build_context_t& ctx) const {
    ctx.query += '(';

    for (size_t i = 0; i < _binders.size(); ++i) {
        if (i != 0)
            ctx.query += ", ";

        ctx.query += ctx.bind(_binders[i]);
    }

    ctx.query += ')';

    return ers::ok;
}

dbio::impl::ClausePtr dbio::impl::ValuesClause::clone() const {
    return std::make_unique<ValuesClause>(_binders);
}
