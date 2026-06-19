#include "erslib/aescript/impl/layout.hpp"

// aescript
#include <erslib/aescript/impl/field.hpp>


aescript::Layout::Layout(std::initializer_list<possible_property_t> il) {
    for (auto& it : il) {
        std::visit([this]<typename T>(const T& v) {
            if constexpr (std::is_same_v<std::decay_t<T>, Field>)
                this->add_field(v);
            else if (std::is_same_v<std::decay_t<T>, DescriptorPtr>)
                this->add_property(v->clone());
        }, it);
    }
}


void aescript::Layout::add_field(Field field) {
    _fields.emplace(std::move(field));
}

void aescript::Layout::add_property(DescriptorPtr property) {
    _descriptors.emplace_back(std::move(property));
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
