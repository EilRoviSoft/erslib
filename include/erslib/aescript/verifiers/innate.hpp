#pragma once

// aescript
#include <erslib/aescript/impl/trait.hpp>
#include <erslib/aescript/impl/verifier.hpp>


template<aescript::internal::SolInnateType T>
struct aescript::verifier_t<T> {
    ers::Status exec(verify_context&, sol::object obj) const {
        return internal::check_type<T>(obj);
    }
};
