#pragma once

// ers
#include <erslib/luaschema/impl/trait.hpp>
#include <erslib/luaschema/impl/verifier.hpp>


template<ers::impl::luaschema::SolInnateType T>
struct ers::impl::luaschema::verifier_t<T> {
    Status exec(verify_context&, sol::object obj) const {
        return luaschema::check_type<T>(obj);
    }
};
