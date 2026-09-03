#pragma once

// ers
#include <erslib/luaschema/impl/trait.hpp>
#include <erslib/luaschema/impl/verifier.hpp>


template<ers::luaschema::impl::HasLayoutDescriptor T>
struct ers::luaschema::verifier_t<T> {
    Status exec(impl::verify_context&, sol::object obj) const {
        if (!obj.is<sol::table>())
            return make_error("Expected table");
        return T::get_layout().verify(obj.as<sol::table>());
    }
};
