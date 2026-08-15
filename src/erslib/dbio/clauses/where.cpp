#include "erslib/dbio/clauses/where.hpp"

// std
#include <string_view>

// ers
#include <erslib/dbio/impl/identity.hpp>
#include <erslib/dbio/slots/where.hpp>


// Internal

namespace {
    // Empty means "unknown", which the callers turn into an error.
    std::string_view op_text(dbio::impl::Op op) {
        using enum dbio::impl::Op;

        switch (op) {
            case Eq:    return " = ";
            case Ne:    return " <> ";
            case Lt:    return " < ";
            case Le:    return " <= ";
            case Gt:    return " > ";
            case Ge:    return " >= ";
            case Like:  return " LIKE ";
            case ILike: return " ILIKE ";
        }

        return {};
    }

    ers::Status render_list(dbio::impl::Context& ctx, const std::vector<dbio::impl::binder_t>& binders) {
        bool first = true;

        for (const auto& it : binders) {
            if (!first)
                ctx.query += ", ";
            first = false;

            ctx.query += it ? ctx.bind(it) : ctx.bind_null();
        }

        return ers::ok;
    }
}


// IWhereClause

dbio::impl::IWhereClause::IWhereClause(SlotRef slot) :
    IClause(slot) {
}


// WhereOpClause

dbio::impl::WhereOpClause::WhereOpClause(std::string column, Op op, binder_t binder) :
    IWhereClause(&slots::where),
    _column(std::move(column)),
    _op(op),
    _binder(std::move(binder)) {
}

ers::Status dbio::impl::WhereOpClause::render(Context& ctx) const {
    if (!is_identifier(_column)) {
        return ers::make_error("Invalid identifier '{}' in slot '{}'.",
            _column, slot()->name);
    }

    const auto op = op_text(_op);
    if (op.empty()) {
        return ers::make_error("Unknown operator ({}) for column '{}' in slot '{}'.",
            static_cast<uint8_t>(_op), _column, slot()->name);
    }

    ctx.query += _column;
    ctx.query += op;
    ctx.query += _binder ? ctx.bind(_binder) : ctx.bind_null();

    return ers::ok;
}


// WhereInClause

dbio::impl::WhereInClause::WhereInClause(std::string column, std::vector<binder_t> binders, bool negated) :
    IWhereClause(&slots::where),
    _column(std::move(column)),
    _binders(std::move(binders)),
    _negated(negated) {
}

ers::Status dbio::impl::WhereInClause::render(Context& ctx) const {
    if (!is_identifier(_column)) {
        return ers::make_error("Invalid identifier '{}' in slot '{}'.",
            _column, slot()->name);
    }

    // "x IN ()" is a syntax error
    if (_binders.empty()) {
        ctx.query += _negated ? "TRUE" : "FALSE";
        return ers::ok;
    }

    ctx.query += _column;
    ctx.query += _negated ? " NOT IN (" : " IN (";

    if (auto s = render_list(ctx, _binders); !s)
        return s;

    ctx.query += ')';

    return ers::ok;
}


// WhereNullClause

dbio::impl::WhereNullClause::WhereNullClause(std::string column, bool is_null) :
    IWhereClause(&slots::where),
    _column(std::move(column)),
    _is_null(is_null) {
}

ers::Status dbio::impl::WhereNullClause::render(Context& ctx) const {
    if (!is_identifier(_column)) {
        return ers::make_error("Invalid identifier '{}' in slot '{}'.",
            _column, slot()->name);
    }

    ctx.query += _column;
    ctx.query += _is_null ? " IS NULL" : " IS NOT NULL";

    return ers::ok;
}


// Shortcuts

dbio::impl::ClausePtr dbio::impl::clauses::where_null(std::string column, bool is_null) {
    return make_clause<WhereNullClause>(std::move(column), is_null);
}
