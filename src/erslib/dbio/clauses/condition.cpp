#include "erslib/dbio/clauses/condition.hpp"


namespace {
    std::string_view to_sql(dbio::impl::Op op) {
        switch (op) {
            case dbio::impl::Op::Eq: return "=";
            case dbio::impl::Op::Ne: return "<>";
            case dbio::impl::Op::Lt: return "<";
            case dbio::impl::Op::Le: return "<=";
            case dbio::impl::Op::Gt: return ">";
            case dbio::impl::Op::Ge: return ">=";
            case dbio::impl::Op::Like: return "LIKE";
            case dbio::impl::Op::ILike: return "ILIKE";
            default: return "=";
        }
    }
}


dbio::impl::ConditionClause::ConditionClause(std::string column, Op op, binder_t binder) :
    IClause(section::where),
    _column(std::move(column)),
    _op(op),
    _binder(std::move(binder)) {
}

ers::Status dbio::impl::ConditionClause::render(build_context_t& ctx) const {
    if (!is_identifier(_column))
        return ers::make_error("Invalid column name '{}'", _column);

    ctx.query += _column;
    ctx.query += ' ';
    ctx.query += to_sql(_op);
    ctx.query += ' ';
    ctx.query += ctx.bind(_binder);

    return ers::ok;
}

dbio::impl::ClausePtr dbio::impl::ConditionClause::clone() const {
    return std::make_unique<ConditionClause>(_column, _op, _binder);
}
