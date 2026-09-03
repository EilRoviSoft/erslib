#pragma once

// std
#include <variant>

// ers
#include <erslib/luaschema/impl/context.hpp>
#include <erslib/luaschema/impl/parser.hpp>
#include <erslib/luaschema/parsers/innate.hpp>


template<typename... Ts>
struct ers::luaschema::parser_t<std::variant<Ts...>> {
    Status exec(impl::parser_context& ctx, sol::object obj, std::variant<Ts...>& dst) const {
        bool matched = (this->_try_one<Ts>(ctx, obj, dst) || ...);
        if (!matched)
            return make_error("No variant alternative matched");
        return ok;
    }


private:
    template<typename T>
    static bool _try_one(impl::parser_context& ctx, sol::object obj, std::variant<Ts...>& dst) {
        T value {};

        if (!parser_t<T> {}.exec(ctx, obj, value))
            return false;
        dst = std::move(value);

        return true;
    }
};
