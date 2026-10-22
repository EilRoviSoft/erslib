#include "erslib/flux/query/clauses/assign.hpp"

// ers
#include <erslib/flux/query/render.hpp>
#include <erslib/flux/query/slots/set.hpp>


// Implementation --------------------------------------------------------------

flux::impl::AssignClause::AssignClause(std::string column, Expr value) :
    IClause(&slots::set),
    _column(std::move(column)),
    _value(std::move(value)) {
}

ers::Status flux::impl::AssignClause::render(Context& ctx) const {
    ERS_TRY(append_identifier(ctx, _column, slot()));

    ctx.query += " = ";
    return _value.render(ctx);
}


// Shortcuts

flux::impl::Clause flux::impl::clauses::assign_null(std::string column) {
    return make_clause<AssignClause>(std::move(column), null());
}
