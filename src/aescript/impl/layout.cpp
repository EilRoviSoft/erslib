#include "aescript/impl/layout.hpp"

// aescript
#include <aescript/impl/field.hpp>


aescript::Layout::Layout(std::initializer_list<std::pair<std::string_view, Field>> il) {
    for (auto& [name, descriptor] : il)
        add_field(static_cast<std::string>(name), descriptor);
}

void aescript::Layout::add_field(std::string name, Field field) {
    _fields.emplace(std::move(name), std::move(field));
}

ers::Status aescript::Layout::verify(sol::table table) const {
    for (const auto& [field, descriptor] : _fields) {
        if (auto r = descriptor.verify(table, field); !r)
            return r;
    }

    return ers::ok;
}
