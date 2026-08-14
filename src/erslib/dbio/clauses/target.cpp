#include "erslib/dbio/clauses/target.hpp"

// ers
#include <erslib/dbio/impl/identity.hpp>


// IdentClause

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

dbio::impl::ClausePtr dbio::impl::IdentityClause::clone() const {
    return std::make_unique<IdentityClause>(slot(), _name);
}


// Shortcuts

dbio::impl::ClausePtr dbio::impl::clauses::identifier(SlotRef slot, std::string name) {
    return std::make_unique<IdentityClause>(slot, std::move(name));
}
