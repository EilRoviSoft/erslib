#include "erslib/dbio/clauses/expr.hpp"

// ers
#include <erslib/dbio/clauses/raw.hpp>
#include <erslib/dbio/slots/column.hpp>


// AliasClause

dbio::impl::AliasClause::AliasClause(Clause inner, std::string alias) :
    IClause(inner->slot()),
    _inner(std::move(inner)),
    _alias(std::move(alias)) {
}

ers::Status dbio::impl::AliasClause::render(Context& ctx) const {
    if (auto s = _inner->render(ctx); !s)
        return s;

    if (auto s = check_identifier(_alias, slot()); !s)
        return s;

    ctx.query += " AS ";
    ctx.query += _alias;

    return ers::ok;
}


dbio::impl::Clause dbio::impl::clauses::named(Clause inner, std::string alias) {
    return make_clause<AliasClause>(std::move(inner), std::move(alias));
}


// CoalesceClause

dbio::impl::CoalesceClause::CoalesceClause(Clause inner, Clause fallback) :
    IClause(inner->slot()),
    _inner(std::move(inner)),
    _fallback(std::move(fallback)) {
}

ers::Status dbio::impl::CoalesceClause::render(Context& ctx) const {
    ctx.query += "coalesce(";

    if (auto s = _inner->render(ctx); !s)
        return s;

    ctx.query += ", ";

    if (auto s = _fallback->render(ctx); !s)
        return s;

    ctx.query += ')';

    return ers::ok;
}


dbio::impl::Clause dbio::impl::clauses::coalesce_with(Clause inner, Clause fallback) {
    return make_clause<CoalesceClause>(std::move(inner), std::move(fallback));
}


// SubqueryClause

dbio::impl::SubqueryClause::SubqueryClause(QueryBuilder subquery, bool exists) :
    IClause(&slots::column),
    _subquery(std::move(subquery)),
    _exists(exists) {
}

ers::Status dbio::impl::SubqueryClause::render(Context& ctx) const {
    return append_subquery(ctx, _subquery, _exists ? "EXISTS (" : "(");
}


dbio::impl::Clause dbio::impl::clauses::subquery(QueryBuilder subquery) {
    return make_clause<SubqueryClause>(std::move(subquery), false);
}

dbio::impl::Clause dbio::impl::clauses::subquery_exists(QueryBuilder subquery) {
    return make_clause<SubqueryClause>(std::move(subquery), true);
}


// Literals

dbio::impl::Clause dbio::impl::clauses::sql_bool(bool value, SlotRef slot) {
    return raw(slot ? slot : &slots::column, value ? "TRUE" : "FALSE");
}
