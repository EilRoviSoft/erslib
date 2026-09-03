#pragma once

// ers
#include <erslib/luaschema/impl/trait.hpp>
#include <erslib/luaschema/impl/parser.hpp>


template<ers::luaschema::impl::SolInnateType T>
struct ers::luaschema::parser_t<T> {
    Status exec(
        impl::parser_context& /*ctx*/,
        sol::object obj,
        T& dst
    ) const {
        if (auto s = impl::check_type<T>(obj); !s)
            return s;

        dst = obj.as<T>();

        return ok;
    }
};
