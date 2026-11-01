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

// assign.cpp
ers::Status flux::impl::AssignClause::render(Context& ctx) const {
    ERS_TRY(render_column(ctx));

    ctx.query += " = ";
    return render_value(ctx);
}


ers::Status flux::impl::AssignClause::render_column(Context& ctx) const {
    return append_identifier(ctx, _column, slot());
}

ers::Status flux::impl::AssignClause::render_value(Context& ctx) const {
    return _value.render(ctx);
}


// Shortcuts

flux::impl::Clause flux::impl::clauses::assign_null(std::string column) {
    return make_clause<AssignClause>(std::move(column), null());
}
