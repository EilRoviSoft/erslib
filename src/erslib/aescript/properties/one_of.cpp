#include "erslib/aescript/properties/one_of.hpp"

// ers
#include <erslib/core/formatter/range.hpp>


aescript::impl::OneOfProperty::OneOfProperty(std::vector<std::string> names) :
    _names(std::move(names)) {
}

ers::Status aescript::impl::OneOfProperty::verify(sol::table table) const {
    std::vector<std::string_view> present;

    present.reserve(_names.size());
    for (const auto& n : _names) {
        if (table.get<std::optional<sol::object>>(n))
            present.emplace_back(n);
    }

    if (present.size() > 1) {
        return ers::make_error("Exactly one of fields {} must be defined, but several are: {}",
            _names, present);
    }

    if (present.empty()) {
        return ers::make_error("Exactly one of fields {} must be defined, but none is",
            _names);
    }

    return ers::ok;
}

aescript::impl::DescriptorPtr aescript::impl::OneOfProperty::clone() const {
    return std::make_unique<OneOfProperty>(_names);
}

aescript::impl::DescriptorPtr aescript::impl::properties::one_of(std::initializer_list<std::string_view> il) {
    std::vector<std::string> names;

    names.reserve(il.size());
    for (const auto& it : il)
        names.emplace_back(it);

    return std::make_unique<OneOfProperty>(std::move(names));
}
