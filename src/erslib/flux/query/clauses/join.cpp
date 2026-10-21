#include "erslib/flux/query/clauses/join.hpp"

// std
#include <utility>

// ers
#include <erslib/flux/query/render.hpp>
#include <erslib/flux/query/slots/join.hpp>


flux::impl::JoinClause::JoinClause(std::string table, std::string alias, std::string left, std::string right, EOp op) :
    IClause(&slots::join),
    _table(std::move(table)),
    _alias(std::move(alias)),
    _left(std::move(left)),
    _right(std::move(right)),
    _op(op) {
}

ers::Status flux::impl::JoinClause::render(Context& ctx) const {
    for (auto* s : { &_table, &_left, &_right }) {
        if (auto st = check_identifier(*s, slot()); !st)
            return st;
    }

    if (!_alias.empty()) {
        if (auto st = check_identifier(_alias, slot()); !st)
            return st;
    }

    ctx.query += "\nJOIN ";
    ctx.query += _table;
    
    if (!_alias.empty()) {
        ctx.query += " AS ";
        ctx.query += _alias;
    }

    ctx.query += " ON ";
    ctx.query += _left;
    ctx.query += ' ';
    ctx.query += to_string(_op);
    ctx.query += ' ';
    ctx.query += _right;

    return ers::ok;
}


flux::impl::Clause flux::impl::clauses::join(std::string table, std::string alias, std::string left, EOp op, std::string right) {
    return make_clause<JoinClause>(std::move(table), std::move(alias), std::move(left), std::move(right), op);
}

flux::impl::Clause flux::impl::clauses::join(std::string table, std::string left, EOp op, std::string right) {
    return make_clause<JoinClause>(std::move(table), "", std::move(left), std::move(right), op);
}
