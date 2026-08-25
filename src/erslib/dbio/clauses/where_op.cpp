#include "erslib/dbio/clauses/where_op.hpp"

// std
#include <format>


dbio::impl::WhereOpClause::WhereOpClause(std::string column, Op op, binder_t binder) :
    IWhereClause(&slots::where),
    _column(std::move(column)),
    _op(op),
    _binder(std::move(binder)) {
}

ers::Status dbio::impl::WhereOpClause::render(Context& ctx) const {
    auto op = op_sql(_op);
    if (op.empty()) {
        return ers::make_error("Unknown operator ({}) for column '{}' in slot '{}'.",
            static_cast<u8>(_op), _column, slot()->name);
    }

    if (auto s = append_identifier(ctx, _column, slot()); !s)
        return s;

    ctx.query += std::format(" {} {}",
        op, ctx.bind(_binder));

    return ers::ok;
}
