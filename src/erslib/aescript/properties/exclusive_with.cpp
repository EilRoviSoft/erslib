#include "erslib/aescript/properties/exclusive_with.hpp"


aescript::ExclusiveWithProperty::ExclusiveWithProperty(std::vector<std::string> incompatible_fields) :
    IVerifier(1),
    _incompatible_fields(std::move(incompatible_fields)) {
}


ers::Status aescript::ExclusiveWithProperty::exec([[maybe_unused]] verify_context& ctx, sol::table table, std::string_view field) const {
    if (!table.get<std::optional<sol::object>>(field))
        return ers::ok;


    std::vector<std::string_view> intersection;

    for (const auto& it : _incompatible_fields) {
        if (table.get<std::optional<sol::object>>(it))
            intersection.emplace_back(it);
    }


    if (!intersection.empty()) {
        return ers::make_error(
            ers::Severity::Error,
            "Field '{}' is exclusive with fields {}",
            field, intersection
        );
    }


    return ers::ok;
}

aescript::VerifierPtr aescript::ExclusiveWithProperty::clone() const {
    return std::make_unique<ExclusiveWithProperty>(_incompatible_fields);
}


aescript::VerifierPtr aescript::properties::exclusive_with(std::string_view field) {
    return exclusive_with({ field });
}

aescript::VerifierPtr aescript::properties::exclusive_with(std::initializer_list<std::string_view> il) {
    std::vector<std::string> incompatible_fields;

    incompatible_fields.reserve(il.size());
    for (const auto& it : il)
        incompatible_fields.emplace_back(it);

    return std::make_unique<ExclusiveWithProperty>(std::move(incompatible_fields));
}
