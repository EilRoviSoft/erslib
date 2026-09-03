#include "erslib/conduit/impl/clause.hpp"

// ers
#include <erslib/conduit/impl/identity.hpp>


// IClause

conduit::impl::IClause::IClause(SlotRef slot) :
    _slot(slot) {
}


// Utility

ers::Status conduit::impl::check_identifier(std::string_view name, SlotRef slot) {
    if (!is_identifier(name)) {
        return ers::make_error("Invalid identifier '{}' in slot '{}'.",
            name, slot ? slot->name : "<null>");
    }

    return ers::ok;
}

ers::Status conduit::impl::append_identifier(Context& ctx, std::string_view name, SlotRef slot) {
    if (auto s = check_identifier(name, slot); !s)
        return s;

    ctx.query += name;
    return ers::ok;
}


ers::Status conduit::impl::append_binders(Context& ctx, std::span<const binder_t> binders, std::string_view separator) {
    return append_joined(ctx, binders, separator, [](Context& ctx, const binder_t& binder) -> ers::Status {
        ctx.query += ctx.bind(binder);
        return ers::ok;
    });
}
