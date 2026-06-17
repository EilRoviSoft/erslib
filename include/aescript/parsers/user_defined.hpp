#pragma once

// aescript
#include <aescript/impl/parser.hpp>
#include <aescript/impl/trait.hpp>


template<typename T>
    requires aescript::internal::HasLayoutDescriptor<T>
struct aescript::parser_t<T> {
    [[nodiscard]]
    ers::Status exec([[maybe_unused]] parser_context& ctx, sol::object obj, T& dst) const {
        if (!obj.is<sol::table>())
            return ers::make_error(ers::Severity::Error, "Expected table");

        return T::get_layout().parse(obj.as<sol::table>(), &dst);
    }
};
