#include "erslib/dbio/clauses/conflict.hpp"

// ers
#include <erslib/dbio/clauses/raw.hpp>


dbio::impl::ConflictUpdateClause::ConflictUpdateClause(std::vector<std::string> columns) :
    IClause(&slots::conflict_action),
    _columns(std::move(columns)) {
}

ers::Status dbio::impl::ConflictUpdateClause::render(Context& ctx) const {
    if (_columns.empty()) {
        return ers::make_error("Slot '{}' needs at least one column.",
            slot()->name);
    }

    ctx.query += "DO UPDATE SET ";

    return append_joined(ctx, _columns, ", ",
        [this](Context& ctx, const std::string& name) -> ers::Status {
            if (auto s = append_identifier(ctx, name, slot()); !s)
                return s;

            ctx.query += " = excluded.";
            ctx.query += name;

            return ers::ok;
        }
    );
}


dbio::impl::Clause dbio::impl::clauses::on_conflict_constraint(std::string name) {
    return identifier(&slots::conflict_constraint, std::move(name));
}

dbio::impl::Clause dbio::impl::clauses::do_nothing() {
    return raw(&slots::conflict_action, "DO NOTHING");
}
