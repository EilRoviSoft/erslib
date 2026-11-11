#include "erslib/luaschema/properties/presence.hpp"

// std
#include <optional>


ers::impl::luaschema::PresenceProperty::PresenceProperty(bool is_required) :
    IVerifier(0),
    _is_required(is_required) {
}

ers::Status ers::impl::luaschema::PresenceProperty::exec([[maybe_unused]] verify_context& ctx, sol::table table, std::string_view field) const {
    if (!table.get<std::optional<sol::object>>(field)) {
        if (_is_required) {
            return ers::make_error("Field '{}' is required but is not found",
                field);
        } else {
            ctx.skip = true;
        }
    }

    return ers::ok;
}


ers::impl::luaschema::Verifier ers::impl::luaschema::properties::presence(bool flag) {
    return make_verifier<PresenceProperty>(flag);
}
