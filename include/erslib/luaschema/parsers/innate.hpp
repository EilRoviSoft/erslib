#pragma once

// ers
#include <erslib/luaschema/impl/trait.hpp>
#include <erslib/luaschema/impl/parser.hpp>


template<ers::impl::luaschema::SolInnateType T>
struct ers::luaschema::parser_t<T> {
    Status exec(
        impl::luaschema::parser_context& /*ctx*/,
        sol::object obj,
        T& dst
    ) const {
        if (auto s = impl::luaschema::check_type<T>(obj); !s)
            return s;

        dst = obj.as<T>();

        return ok;
    }
};
