#pragma once

// ers
#include <erslib/luaschema/impl/parser.hpp>
#include <erslib/luaschema/impl/trait.hpp>


template<typename T>
    requires ers::impl::luaschema::HasLayoutDescriptor<T>
struct ers::luaschema::parser_t<T> {
    Status exec(impl::luaschema::parser_context& /*ctx*/, sol::object obj, T& dst) const {
        if (!obj.is<sol::table>())
            return make_error("Expected table");

        return T::get_layout().parse(obj.as<sol::table>(), &dst);
    }
};
