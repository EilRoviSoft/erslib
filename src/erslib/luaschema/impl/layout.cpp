#include "erslib/luaschema/impl/layout.hpp"

// ers
#include <erslib/luaschema/impl/field.hpp>


ers::luaschema::impl::Layout::Layout(std::initializer_list<possible_property_t> il) {
    for (auto& it : il) {
        std::visit([this]<typename T>(const T& v) {
            if constexpr (std::is_same_v<std::decay_t<T>, Field>)
                this->add_field(v);
            else if (std::is_same_v<std::decay_t<T>, Descriptor>)
                this->add_property(v);
        }, it);
    }
}


void ers::luaschema::impl::Layout::add_field(Field field) {
    _fields.emplace(std::move(field));
}

void ers::luaschema::impl::Layout::add_property(Descriptor property) {
    _descriptors.emplace_back(std::move(property));
}


ers::Status ers::luaschema::impl::Layout::verify(sol::table table) const {
    for (const auto& field : _fields) {
        if (auto r = field.verify(table); !r)
            return r;
    }

    return ers::ok;
}

ers::Status ers::luaschema::impl::Layout::parse(sol::table table, void* where) const {
    for (const auto& field : _fields) {
        if (auto s = field.parse(table, where); !s) {
            return s;
        }
    }

    return ers::ok;
}
