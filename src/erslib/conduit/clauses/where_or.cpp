#include "erslib/conduit/clauses/where_or.hpp"

// ers
#include <erslib/conduit/slots/where.hpp>


conduit::impl::WhereOrClause::WhereOrClause(std::vector<Clause> branches) :
    IWhereClause(&slots::where),
    _branches(std::move(branches)) {
}


ers::Status conduit::impl::WhereOrClause::render(Context& ctx) const {
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
