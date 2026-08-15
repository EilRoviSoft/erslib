#include "erslib/dbio/clauses/target.hpp"

// ers
#include <erslib/dbio/impl/identity.hpp>
#include <erslib/dbio/slots/column.hpp>
#include <erslib/dbio/slots/from.hpp>


// IdentityClause

dbio::impl::IdentityClause::IdentityClause(SlotRef slot, std::string name) :
    IClause(slot),
    _name(std::move(name)) {
}

ers::Status dbio::impl::IdentityClause::render(Context& ctx) const {
    if (!is_identifier(_name)) {
        return ers::make_error("Invalid identifier '{}' in slot '{}'.",
            _name, slot()->name);
    }

    ctx.query += _name;

    return ers::ok;
}


// Shortcuts

dbio::impl::ClausePtr dbio::impl::clauses::identifier(SlotRef slot, std::string name) {
    return make_clause<IdentityClause>(slot, std::move(name));
}

dbio::impl::ClausePtr dbio::impl::clauses::column(std::string name) {
    return identifier(&slots::column, std::move(name));
}

dbio::impl::ClausePtr dbio::impl::clauses::from(std::string name) {
    return identifier(&slots::from, std::move(name));
}
