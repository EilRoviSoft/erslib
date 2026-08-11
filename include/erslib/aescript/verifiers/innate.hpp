#pragma once

// ers
#include <erslib/aescript/impl/trait.hpp>
#include <erslib/aescript/impl/verifier.hpp>


template<aescript::impl::SolInnateType T>
struct aescript::verifier_t<T> {
    ers::Status exec(impl::verify_context&, sol::object obj) const {
        return impl::check_type<T>(obj);
    }
};
