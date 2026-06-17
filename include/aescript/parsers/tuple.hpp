#pragma once

// std
#include <tuple>

// aescript
#include <aescript/impl/context.hpp>
#include <aescript/impl/parser.hpp>
#include <aescript/impl/parsers/innate.hpp>


template<typename... Ts>
struct aescript::parser_t<std::variant<Ts...>> {
    [[nodiscard]]
    ers::Status exec(parser_context& ctx, sol::object obj, std::variant<Ts...>& dst) const {
        if (!obj.is<sol::table>())
            return ers::make_error(ers::Severity::Error, "Expected array table for tuple");
        return _impl(ctx, obj.as<sol::table>(), dst, std::index_sequence_for<Ts...> {});
    }


private:
    template<size_t... Is>
    static ers::Status _impl(
        parser_context& ctx,
        sol::table arr,
        std::tuple<Ts...>& dst, std::index_sequence<Is...>
    ) {
        ers::Status s = ers::ok;
        ((s = parser_t<Ts> {}.exec(ctx, arr[Is + 1], std::get<Is>(dst))) && ...);
        return s;
    }
};
