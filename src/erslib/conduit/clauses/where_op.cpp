#include "erslib/conduit/clauses/where_op.hpp"

// ers
#include <erslib/conduit/slots/where.hpp>


conduit::impl::WhereOpClause::WhereOpClause(std::string column, EOp op, binder_t binder) :
    IWhereClause(&slots::where),
    _column(std::move(column)),
    _op(op),
    _binder(std::move(binder)) {
}

ers::Status conduit::impl::WhereOpClause::render(Context& ctx) const {
    auto op_str = to_string(_op);
    if (op_str.empty()) {
        return ers::make_error("Unknown operator ({}) for column '{}' in slot '{}'.",
            static_cast<u8>(_op), _column, slot()->name);
    }

    if (auto s = append_identifier(ctx, _column, slot()); !s)
        return s;

    ctx.query += ' ';
    ctx.query += op_str;
    ctx.query += ' ';
    ctx.query += ctx.bind(_binder);

    return ers::ok;
}
