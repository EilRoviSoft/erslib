#include "erslib/dbio/clauses/target.hpp"

// ers
#include <erslib/dbio/slots/column.hpp>
#include <erslib/dbio/slots/from.hpp>


// IdentityClause

dbio::impl::IdentityClause::IdentityClause(SlotRef slot, std::string name) :
    IClause(slot),
    _name(std::move(name)) {
}

ers::Status dbio::impl::IdentityClause::render(Context& ctx) const {
    return append_identifier(ctx, _name, slot());
}


// Shortcuts

dbio::impl::Clause dbio::impl::clauses::identifier(SlotRef slot, std::string name) {
    return make_clause<IdentityClause>(slot, std::move(name));
}

dbio::impl::Clause dbio::impl::clauses::column(std::string name) {
    return identifier(&slots::column, std::move(name));
}

dbio::impl::Clause dbio::impl::clauses::from(std::string name) {
    return identifier(&slots::from, std::move(name));
}
