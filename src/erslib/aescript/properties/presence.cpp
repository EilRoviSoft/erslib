#include "erslib/aescript/properties/presence.hpp"


aescript::PresenceProperty::PresenceProperty(bool is_required) :
    IVerifier(0),
    _is_required(is_required) {
}

ers::Status aescript::PresenceProperty::exec([[maybe_unused]] verify_context& ctx, sol::table table, std::string_view field) const {
    if (!table.get<std::optional<sol::object>>(field)) {
        if (_is_required) {
            return ers::make_error(
                ers::Severity::Error,
                "Field '{}' is required but is not found",
                field
            );
        } else {
            ctx.skip = true;
        }
    }

    return ers::ok;
}

aescript::VerifierPtr aescript::PresenceProperty::clone() const {
    return std::make_unique<PresenceProperty>(_is_required);
}


aescript::VerifierPtr aescript::properties::presence(bool flag) {
    return std::make_unique<PresenceProperty>(flag);
}
