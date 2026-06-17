#include "aescript/impl/properties/exclusive_with.hpp"


aescript::ExclusiveWithProperty::ExclusiveWithProperty(std::initializer_list<std::string_view> il) :
    ExclusiveWithProperty() {
    for (const auto& it : il)
        _incompatible_fields.emplace_back(it);
}

aescript::ExclusiveWithProperty::ExclusiveWithProperty() :
    IProperty(1) {
}


ers::Status aescript::ExclusiveWithProperty::verify([[maybe_unused]] property_context& ctx, sol::table table, std::string_view field) const {
    if (!table.get<std::optional<sol::object>>(field))
        return ers::ok;


    std::list<std::string_view> intersection;

    for (const auto& it : _incompatible_fields) {
        if (table.get<std::optional<sol::object>>(it))
            intersection.emplace_back(it);
    }


    if (!intersection.empty()) {
        return ers::make_error(
            ers::Severity::Error,
            "Field '{}' is exclusive with fields [{}]",
            field, intersection
        );
    }


    return ers::ok;
}

aescript::FieldPropertyPtr aescript::ExclusiveWithProperty::clone() const {
    ExclusiveWithProperty result;
    result._incompatible_fields = _incompatible_fields;
    return std::make_unique<ExclusiveWithProperty>(std::move(result));
}


aescript::FieldPropertyPtr aescript::properties::exclusive_with(std::initializer_list<std::string_view> il) {
    return std::make_unique<ExclusiveWithProperty>(il);
}
