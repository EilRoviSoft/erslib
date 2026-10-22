#include "erslib/flux/query/clauses/column.hpp"

// ers
#include <erslib/flux/query/render.hpp>


// Implementation --------------------------------------------------------------

flux::impl::ColumnClause::ColumnClause(SlotRef slot, Expr what, std::string alias) :
    IClause(slot),
    _what(std::move(what)),
    _alias(std::move(alias)) {
}

ers::Status flux::impl::ColumnClause::render(Context& ctx) const {
    ERS_TRY(_what.render(ctx));

    if (_alias.empty())
        return ers::ok;

    ERS_TRY(check_identifier(_alias, slot()));

    ctx.query += " AS ";
    ctx.query += _alias;
    return ers::ok;
}


flux::impl::Clause flux::impl::make_column(SlotRef slot, ColumnExpr what) {
    return make_clause<ColumnClause>(slot, std::move(what.expr));
}

flux::impl::Clause flux::impl::make_column(SlotRef slot, Aliased what) {
    return make_clause<ColumnClause>(slot, std::move(what.expr), std::move(what.alias));
}


// Shortcuts

flux::impl::Clause flux::impl::clauses::column(ColumnExpr what) {
    return make_column(&slots::column, std::move(what));
}

flux::impl::Clause flux::impl::clauses::column(Aliased what) {
    return make_column(&slots::column, std::move(what));
}
