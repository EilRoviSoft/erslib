#include "erslib/flux/clauses/where_column.hpp"

// std
#include <format>

// ers
#include <erslib/flux/slots/where.hpp>


flux::impl::WhereColumnClause::WhereColumnClause(std::string left, std::string right, EOp op) :
    IWhereClause(&slots::where),
    _left(std::move(left)),
    _right(std::move(right)),
    _op(op) {
}

ers::Status flux::impl::WhereColumnClause::render(Context& ctx) const {
    if (auto s = check_identifier(_left, slot()); !s)
        return s;

    if (auto s = check_identifier(_right, slot()); !s)
        return s;

    ctx.query += std::format("{} {} {}",
        _left, to_string(_op), _right);

    return ers::ok;
}


flux::impl::Clause flux::impl::clauses::where_column(std::string left, EOp op, std::string right) {
    return make_clause<WhereColumnClause>(std::move(left), std::move(right), op);
}
