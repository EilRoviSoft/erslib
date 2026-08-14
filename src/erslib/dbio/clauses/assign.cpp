#include "erslib/dbio/clauses/assign.hpp"

// ers
#include <erslib/dbio/slots/set.hpp>


dbio::impl::AssignClause::AssignClause(std::string column, binder_t binder) :
    IClause(&slots::set),
    _column(std::move(column)),
    _binder(std::move(binder)) {
}

ers::Status dbio::impl::AssignClause::render(Context& ctx) const {
    if (!is_identifier(_column)) {
        return ers::make_error("Invalid identifier '{}' in slot '{}'.",
            _column, slot()->name);
    }

    ctx.query += _column;
    ctx.query += " = ";
    ctx.query += _binder ? ctx.bind(_binder) : ctx.bind_null();

    return ers::ok;
}

dbio::impl::ClausePtr dbio::impl::AssignClause::clone() const {
    return std::make_unique<AssignClause>(_column, _binder);
}


dbio::impl::ClausePtr dbio::impl::clauses::assign_null(std::string column) {
    return std::make_unique<AssignClause>(std::move(column), binder_t {});
}
