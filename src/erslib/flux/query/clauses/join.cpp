#include "erslib/flux/query/clauses/join.hpp"

// ers
#include <erslib/flux/query/render.hpp>
#include <erslib/flux/query/slots/join.hpp>


flux::impl::JoinClause::JoinClause(std::string table, std::string alias, Expr on) :
    IClause(&slots::join),
    _table(std::move(table)),
    _alias(std::move(alias)),
    _on(std::move(on)) {
}

ers::Status flux::impl::JoinClause::render(Context& ctx) const {
    ERS_TRY(check_identifier(_table, slot()));

    ctx.query += "\nJOIN ";
    ctx.query += _table;

    if (!_alias.empty()) {
        ERS_TRY(check_identifier(_alias, slot()));

        ctx.query += " AS ";
        ctx.query += _alias;
    }

    ctx.query += " ON ";
    return _on.render(ctx);
}
