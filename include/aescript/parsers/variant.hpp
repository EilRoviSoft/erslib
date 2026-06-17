#pragma once

// std
#include <variant>

// aescript
#include <aescript/impl/context.hpp>
#include <aescript/impl/parser.hpp>
#include <aescript/impl/parsers/innate.hpp>


template<typename... Ts>
struct aescript::parser_t<std::variant<Ts...>> {
    [[nodiscard]]
    ers::Status exec(parser_context& ctx, sol::object obj, std::variant<Ts...>& dst) const {
        bool matched = (_try_one<Ts>(ctx, obj, dst) || ...);
        if (!matched)
            return ers::make_error(ers::Severity::Error, "No variant alternative matched");
        return ers::ok;
    }


private:
    template<typename T>
    static bool _try_one(parser_context& ctx, sol::object obj, std::variant<Ts...>& dst) {
        T value {};

        if (!parser_t<T> {}.exec(ctx, obj, value))
            return false;
        dst = std::move(value);

        return true;
    }
};
