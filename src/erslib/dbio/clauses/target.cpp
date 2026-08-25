#include "erslib/dbio/clauses/target.hpp"

// ers
#include <erslib/dbio/slots/column.hpp>
#include <erslib/dbio/slots/from.hpp>


// IdentityClause

dbio::impl::IdentityClause::IdentityClause(SlotRef slot, std::string name, std::string alias) :
    IClause(slot),
    _name(std::move(name)),
    _alias(std::move(alias)) {
}

ers::Status dbio::impl::IdentityClause::render(Context& ctx) const {
    if (auto s = append_identifier(ctx, _name, slot()); !s)
        return s;

    if (!_alias.empty()) {
        if (auto s = check_identifier(_alias, slot()); !s)
            return s;

        ctx.query += " AS ";
        ctx.query += _alias;
    }

    return ers::ok;
}


// Shortcuts

dbio::impl::Clause dbio::impl::clauses::identifier(SlotRef slot, std::string name, std::string alias) {
    return make_clause<IdentityClause>(slot, std::move(name), std::move(alias));
}

dbio::impl::Clause dbio::impl::clauses::column(std::string name) {
    return identifier(&slots::column, std::move(name));
}

dbio::impl::Clause dbio::impl::clauses::from(std::string name, std::string alias) {
    return identifier(&slots::from, std::move(name), std::move(alias));
}
