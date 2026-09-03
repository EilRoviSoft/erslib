#pragma once

// ers
#include <erslib/luaschema/impl/trait.hpp>
#include <erslib/luaschema/impl/verifier.hpp>


template<ers::luaschema::impl::SolInnateType T>
struct ers::luaschema::verifier_t<T> {
    Status exec(impl::verify_context&, sol::object obj) const {
        return impl::check_type<T>(obj);
    }
};
