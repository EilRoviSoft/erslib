#include "erslib/dbio/clauses/join.hpp"

// ers
#include <erslib/dbio/slots/join.hpp>


dbio::impl::JoinClause::JoinClause(std::string table, std::string alias, std::string left, std::string right, Op op) :
    IClause(&slots::join),
    _table(std::move(table)),
    _alias(std::move(alias)),
    _left(std::move(left)),
    _right(std::move(right)),
    _op(op) {
}

ers::Status dbio::impl::JoinClause::render(Context& ctx) const {
    for (auto* s : { &_table, &_alias, &_left, &_right }) {
        if (auto st = check_identifier(*s, slot()); !st)
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
    ctx.query += op_sql(_op);
    ctx.query += ' ';
    ctx.query += _right;

    return ers::ok;
}


dbio::impl::Clause dbio::impl::clauses::join(std::string table, std::string alias, std::string left, Op op, std::string right) {
    return make_clause<JoinClause>(std::move(table), std::move(alias), std::move(left), std::move(right), op);
}

dbio::impl::Clause dbio::impl::clauses::join(std::string table, std::string alias, std::string left, std::string right) {
    return join(std::move(table), std::move(alias), std::move(left), Op::Eq, std::move(right));
}
