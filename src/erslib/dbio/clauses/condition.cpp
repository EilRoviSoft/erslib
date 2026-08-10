#include "erslib/dbio/clauses/condition.hpp"


namespace {
    std::string_view to_sql(dbio::Op op) {
        switch (op) {
            case dbio::Op::Eq: return "=";
            case dbio::Op::Ne: return "<>";
            case dbio::Op::Lt: return "<";
            case dbio::Op::Le: return "<=";
            case dbio::Op::Gt: return ">";
            case dbio::Op::Ge: return ">=";
            case dbio::Op::Like: return "LIKE";
            case dbio::Op::ILike: return "ILIKE";
            default: return "=";
        }
    }
}


dbio::ConditionClause::ConditionClause(std::string column, Op op, internal::binder_t binder) :
    IClause(Section::Where),
    _column(std::move(column)),
    _op(op),
    _binder(std::move(binder)) {
}

ers::Status dbio::ConditionClause::render(build_context_t& ctx) const {
    if (!internal::is_identifier(_column))
        return ers::make_error("Invalid column name '{}'", _column);

    ctx.query += _column;
    ctx.query += ' ';
    ctx.query += to_sql(_op);
    ctx.query += ' ';
    ctx.query += ctx.bind(_binder);

    return ers::ok;
}

dbio::ClausePtr dbio::ConditionClause::clone() const {
    return std::make_unique<ConditionClause>(_column, _op, _binder);
}
