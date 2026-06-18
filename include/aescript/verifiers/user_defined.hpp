#pragma once

// aescript
#include <aescript/impl/trait.hpp>
#include <aescript/impl/verifier.hpp>


template<aescript::internal::HasLayoutDescriptor T>
struct aescript::verifier_t<T> {
    ers::Status exec(verify_context&, sol::object obj) const {
        if (!obj.is<sol::table>())
            return ers::make_error(ers::Severity::Error, "Expected table");
        return T::get_layout().verify(obj.as<sol::table>());
    }
};
