#pragma once

// ers
#include <erslib/luaschema/impl/trait.hpp>
#include <erslib/luaschema/impl/verifier.hpp>


template<ers::impl::luaschema::SolInnateType T>
struct ers::luaschema::verifier_t<T> {
    Status exec(impl::luaschema::verify_context&, sol::object obj) const {
        return impl::luaschema::check_type<T>(obj);
    }
};
