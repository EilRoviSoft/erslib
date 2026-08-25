#include "erslib/dbio/clauses/where_any.hpp"

// ers
#include <erslib/dbio/slots/where.hpp>


dbio::impl::WhereAnyClause::WhereAnyClause(std::string column, binder_t values, std::string cast, bool guard_empty) :
    IWhereClause(&slots::where),
    _column(std::move(column)),
    _values(std::move(values)),
    _cast(std::move(cast)),
    _guard_empty(guard_empty) {
}

ers::Status dbio::impl::WhereAnyClause::render(Context& ctx) const {
    if (!_column.empty()) {
        if (auto s = check_identifier(_column, slot()); !s)
            return s;
    }

    // Bound once, referenced twice below.
    const auto placeholder = ctx.bind(_values);

    const auto append_cardinality = [&] {
        ctx.query += "cardinality(";
        ctx.query += placeholder;
        ctx.query += "::";
        ctx.query += _cast;
        ctx.query += ") = 0";
    };

    const auto append_any = [&] {
        ctx.query += _column;
        ctx.query += " = ANY(";
        ctx.query += placeholder;
        ctx.query += ')';
    };


    if (_column.empty())
        append_cardinality();
    else if (_guard_empty) {
        ctx.query += '(';
        append_cardinality();
        ctx.query += " OR ";
        append_any();
        ctx.query += ')';
    } else
        append_any();

    return ers::ok;
}
