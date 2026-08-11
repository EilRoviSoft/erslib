#pragma once

// ers
#include <erslib/aescript/impl/trait.hpp>
#include <erslib/aescript/impl/verifier.hpp>


template<aescript::impl::HasLayoutDescriptor T>
struct aescript::verifier_t<T> {
    ers::Status exec(impl::verify_context&, sol::object obj) const {
        if (!obj.is<sol::table>())
            return ers::make_error("Expected table");
        return T::get_layout().verify(obj.as<sol::table>());
    }
};
