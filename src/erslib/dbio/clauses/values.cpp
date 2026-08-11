#include "erslib/dbio/clauses/values.hpp"


dbio::ValuesClause::ValuesClause(std::vector<internal::binder_t> binders) :
    IClause(section::values),
    _binders(std::move(binders)) {
}

ers::Status dbio::ValuesClause::render(build_context_t& ctx) const {
    ctx.query += '(';

    for (size_t i = 0; i < _binders.size(); ++i) {
        if (i != 0)
            ctx.query += ", ";

        ctx.query += ctx.bind(_binders[i]);
    }

    ctx.query += ')';

    return ers::ok;
}

dbio::ClausePtr dbio::ValuesClause::clone() const {
    return std::make_unique<ValuesClause>(_binders);
}
