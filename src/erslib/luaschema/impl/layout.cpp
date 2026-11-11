#include "erslib/luaschema/impl/layout.hpp"

// std
#include <type_traits>
#include <utility>

// ers
#include <erslib/luaschema/impl/field.hpp>


ers::impl::luaschema::Layout::Layout(std::initializer_list<possible_property_t> il) {
    for (auto& it : il) {
        std::visit([this]<typename T>(const T& v) {
            if constexpr (std::is_same_v<std::decay_t<T>, Field>)
                this->add_field(v);
            else if (std::is_same_v<std::decay_t<T>, Descriptor>)
                this->add_property(v);
        }, it);
    }
}


void ers::impl::luaschema::Layout::add_field(Field field) {
    _fields.emplace(std::move(field));
}

void ers::impl::luaschema::Layout::add_property(Descriptor property) {
    _descriptors.push_back(std::move(property));
}


ers::Status ers::impl::luaschema::Layout::verify(sol::table table) const {
    for (const auto& field : _fields) {
        if (auto r = field.verify(table); !r)
            return r;
    }

    return ers::ok;
}

ers::Status ers::impl::luaschema::Layout::parse(sol::table table, void* where) const {
    for (const auto& field : _fields) {
        if (auto s = field.parse(table, where); !s) {
            return s;
        }
    }

    return ers::ok;
}
