#include "erslib/dbio/clauses/where_column.hpp"

// ers
#include <erslib/dbio/slots/where.hpp>


dbio::impl::WhereColumnClause::WhereColumnClause(std::string left, std::string right, Op op) :
    IWhereClause(&slots::where),
    _left(std::move(left)),
    _right(std::move(right)),
    _op(op) {
}

ers::Status dbio::impl::WhereColumnClause::render(Context& ctx) const {
    if (auto s = check_identifier(_left, slot()); !s)
        return s;

    if (auto s = check_identifier(_right, slot()); !s)
        return s;

    ctx.query += std::format("{} {} {}",
        _left, op_sql(_op), _right);

    return ers::ok;
}


dbio::impl::Clause dbio::impl::clauses::where_column(std::string left, Op op, std::string right) {
    return make_clause<WhereColumnClause>(std::move(left), std::move(right), op);
}

dbio::impl::Clause dbio::impl::clauses::where_column(std::string left, std::string right) {
    return make_clause<WhereColumnClause>(std::move(left), std::move(right), Op::Eq);
}
