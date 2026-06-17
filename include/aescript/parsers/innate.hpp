#pragma once

// aescript
#include <aescript/impl/trait.hpp>
#include <aescript/impl/parser.hpp>


template<aescript::internal::SolInnateType T>
struct aescript::parser_t<T> {
    [[nodiscard]]
    ers::Status exec(
        [[maybe_unused]] parser_context& ctx,
        sol::object obj,
        T& dst
    ) const {
        if (auto s = internal::check_type<T>(obj); !s)
            return s;

        dst = obj.as<T>();

        return ers::ok;
    }
};
