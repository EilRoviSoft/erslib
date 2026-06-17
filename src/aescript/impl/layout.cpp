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
    for (const auto& [name, field] : _fields) {
        if (auto r = field.verify(table, name); !r)
            return r;
    }

    return ers::ok;
}

ers::Status aescript::Layout::parse(sol::table table, void* where) const {
    for (const auto& [name, field] : _fields) {
        if (auto s = field.parse(table, name, where); !s) {
            return s;
        }
    }

    return ers::ok;
}
