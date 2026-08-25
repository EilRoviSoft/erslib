#include "erslib/dbio/clauses/assign.hpp"

// ers
#include <erslib/dbio/slots/set.hpp>


// AssignClause

dbio::impl::AssignClause::AssignClause(std::string column, binder_t binder) :
    IClause(&slots::set),
    _column(std::move(column)),
    _binder(std::move(binder)) {
}

ers::Status dbio::impl::AssignClause::render(Context& ctx) const {
    if (auto s = append_identifier(ctx, _column, slot()); !s)
        return s;

    ctx.query += " = ";
    ctx.query += ctx.bind(_binder);

    return ers::ok;
}


// Shortcuts

dbio::impl::Clause dbio::impl::clauses::assign_null(std::string column) {
    return make_clause<AssignClause>(std::move(column), binder_t {});
}
