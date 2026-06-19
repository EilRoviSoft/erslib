#include "erslib/aescript/properties/inclusive_with.hpp"


aescript::InclusiveWithProperty::InclusiveWithProperty(std::vector<std::string> mandatory_fields) :
    IVerifier(1),
    _mandatory_fields(std::move(mandatory_fields)) {
}


ers::Status aescript::InclusiveWithProperty::exec([[maybe_unused]] verify_context& ctx, sol::table table, std::string_view field) const {
    if (!table.get<std::optional<sol::object>>(field))
        return ers::ok;


    std::list<std::string_view> absent;

    for (const auto& it : _mandatory_fields) {
        if (!table.get<std::optional<sol::object>>(it))
            absent.emplace_back(it);
    }


    if (!absent.empty()) {
        return ers::make_error(
            ers::Severity::Error,
            "Field '{}' is inclusive with fields {}",
            field, absent
        );
    }


    return ers::ok;
}

aescript::VerifierPtr aescript::InclusiveWithProperty::clone() const {
    return std::make_unique<InclusiveWithProperty>(_mandatory_fields);
}


aescript::VerifierPtr aescript::properties::inclusive_with(std::string_view field) {
    return inclusive_with({ field });
}

aescript::VerifierPtr aescript::properties::inclusive_with(std::initializer_list<std::string_view> il) {
    std::vector<std::string> mandatory_fields;

    mandatory_fields.reserve(il.size());
    for (const auto& it : il)
        mandatory_fields.emplace_back(it);

    return std::make_unique<InclusiveWithProperty>(std::move(mandatory_fields));
}
