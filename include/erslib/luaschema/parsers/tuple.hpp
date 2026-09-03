#pragma once

// std
#include <tuple>

// ers
#include <erslib/luaschema/impl/context.hpp>
#include <erslib/luaschema/impl/parser.hpp>
#include <erslib/luaschema/parsers/innate.hpp>


template<typename... Ts>
struct ers::luaschema::parser_t<std::variant<Ts...>> {
    Status exec(impl::parser_context& ctx, sol::object obj, std::variant<Ts...>& dst) const {
        if (!obj.is<sol::table>())
            return make_error("Expected array table for tuple");
        return this->_impl(ctx, obj.as<sol::table>(), dst, std::index_sequence_for<Ts...> {});
    }


private:
    template<size_t... Is>
    static Status _impl(
        impl::parser_context& ctx,
        sol::table arr,
        std::tuple<Ts...>& dst, std::index_sequence<Is...>
    ) {
        Status s = ok;
        ((s = parser_t<Ts> {}.exec(ctx, arr[Is + 1], std::get<Is>(dst))) && ...);
        return s;
    }
};
