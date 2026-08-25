#include "erslib/dbio/clauses/where_or.hpp"

// ers
#include <erslib/dbio/slots/where.hpp>


dbio::impl::WhereOrClause::WhereOrClause(std::vector<Clause> branches) :
    IWhereClause(&slots::where),
    _branches(std::move(branches)) {
}


ers::Status dbio::impl::WhereOrClause::render(Context& ctx) const {
    ctx.query += '(';

    bool first = true;
    for (auto& branch : _branches) {
        if (!first) ctx.query += " OR ";
        first = false;

        if (auto s = branch->render(ctx); !s)
            return s;
    }

    ctx.query += ')';
    return ers::ok;
}
