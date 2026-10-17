#include "erslib/flux/query/render.hpp"

// ers
#include <erslib/flux/sql/identifier.hpp>


ers::Status flux::impl::check_identifier(std::string_view name, SlotRef slot) {
    if (!is_identifier(name)) {
        return ers::make_error("Invalid identifier '{}' in slot '{}'.",
            name, slot ? slot->name : "<null>");
    }

    return ers::ok;
}

ers::Status flux::impl::append_identifier(Context& ctx, std::string_view name, SlotRef slot) {
    if (auto s = check_identifier(name, slot); !s)
        return s;

    ctx.query += name;
    return ers::ok;
}


ers::Status flux::impl::append_binders(Context& ctx, std::span<const binder_t> binders, std::string_view separator) {
    return append_joined(ctx, binders, separator, [](Context& ctx, const binder_t& binder) -> ers::Status {
        ctx.query += ctx.bind(binder);
        return ers::ok;
    });
}
