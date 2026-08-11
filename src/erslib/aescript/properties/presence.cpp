#include "erslib/aescript/properties/presence.hpp"


aescript::impl::PresenceProperty::PresenceProperty(bool is_required) :
    IVerifier(0),
    _is_required(is_required) {
}

ers::Status aescript::impl::PresenceProperty::exec([[maybe_unused]] verify_context& ctx, sol::table table, std::string_view field) const {
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

aescript::impl::VerifierPtr aescript::impl::PresenceProperty::clone() const {
    return std::make_unique<PresenceProperty>(_is_required);
}


aescript::impl::VerifierPtr aescript::impl::properties::presence(bool flag) {
    return std::make_unique<PresenceProperty>(flag);
}
