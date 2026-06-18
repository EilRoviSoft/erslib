#include "aescript/impl/layout.hpp"

// aescript
#include <aescript/impl/field.hpp>


aescript::Layout::Layout(std::initializer_list<Field> il) {
    for (auto& field : il)
        add_field(field);
}


void aescript::Layout::add_field(Field field) {
    _fields.emplace(std::move(field));
}


ers::Status aescript::Layout::verify(sol::table table) const {
    for (const auto& field : _fields) {
        if (auto r = field.verify(table); !r)
            return r;
    }

    return ers::ok;
}

ers::Status aescript::Layout::parse(sol::table table, void* where) const {
    for (const auto& field : _fields) {
        if (auto s = field.parse(table, where); !s) {
            return s;
        }
    }

    return ers::ok;
}
