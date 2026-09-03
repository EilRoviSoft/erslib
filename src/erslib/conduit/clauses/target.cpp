#include "erslib/conduit/clauses/target.hpp"

// ers
#include <erslib/conduit/slots/column.hpp>
#include <erslib/conduit/slots/from.hpp>


// IdentityClause

conduit::impl::IdentityClause::IdentityClause(SlotRef slot, std::string name, std::string alias) :
    IClause(slot),
    _name(std::move(name)),
    _alias(std::move(alias)) {
}

ers::Status conduit::impl::IdentityClause::render(Context& ctx) const {
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

conduit::impl::Clause conduit::impl::clauses::identifier(SlotRef slot, std::string name, std::string alias) {
    return make_clause<IdentityClause>(slot, std::move(name), std::move(alias));
}

conduit::impl::Clause conduit::impl::clauses::column(std::string name) {
    return identifier(&slots::column, std::move(name));
}

conduit::impl::Clause conduit::impl::clauses::from(std::string name, std::string alias) {
    return identifier(&slots::from, std::move(name), std::move(alias));
}
